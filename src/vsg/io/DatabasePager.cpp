/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>

#include <iostream>

using namespace vsg;

#define DO_TIMING 0
#define REPORT_STATS 0

/////////////////////////////////////////////////////////////////////////
//
// DatabasePager
//
DatabaseQueue::DatabaseQueue(ref_ptr<ActivityStatus> status) :
    _status(status)
{
}

DatabaseQueue::~DatabaseQueue()
{
}

void DatabaseQueue::add(ref_ptr<PagedLOD> plod)
{
    // std::cout<<"DatabaseQueue::add("<<plod<<") status = "<<plod->requestStatus.load()<<std::endl;

    std::scoped_lock lock(_mutex);
    _queue.emplace_back(plod);
    _cv.notify_one();
}

void DatabaseQueue::add_then_reset(ref_ptr<PagedLOD>& plod)
{
    std::scoped_lock lock(_mutex);
    _queue.emplace_back(plod);
    _cv.notify_one();
    plod = nullptr;
}

void DatabaseQueue::add(Nodes& nodes)
{
    std::scoped_lock lock(_mutex);

    _queue.insert(_queue.end(), nodes.begin(), nodes.end());

    _cv.notify_one();
}

ref_ptr<PagedLOD> DatabaseQueue::take_when_available()
{
    //std::cout<<"DatabaseQueue::take_when_available() A _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;

    std::chrono::duration waitDuration = std::chrono::milliseconds(100);
    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && _status->active())
    {
        //std::cout<<"   Waiting on condition variable B _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (_queue.empty() || _status->cancel())
    {
        //std::cout<<"DatabaseQueue::take_when_available() C _identifier = "<<_identifier<<" empty"<<std::endl;
        return {};
    }

    //std::cout<<"DatabaseQueue::take_when_available() D _identifier = "<<_identifier<<" "<<_queue.size()<<std::endl;

#if 1

    // find the PagedLOD with the highest priority;
    auto itr = _queue.begin();
    auto highest_itr = itr++;

    for (; itr != _queue.end(); ++itr)
    {
        if ((*itr)->priority > (*highest_itr)->priority) highest_itr = itr;
    }

    ref_ptr<PagedLOD> plod = *highest_itr;
    _queue.erase(highest_itr);

    //std::cout<<"Returning "<<plod.get()<<std::dec<<" variable _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;

#else
    // remove and return the head of the queue
    ref_ptr<PagedLOD> plod = _queue.front();
    _queue.erase(_queue.begin());
#endif
    return plod;
}

DatabaseQueue::Nodes DatabaseQueue::take_all_when_available()
{
    std::chrono::duration waitDuration = std::chrono::milliseconds(100);

    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && _status->active())
    {
        //std::cout<<"take_all_when_available() _identifier = "<<_identifier<<" Waiting on condition variable"<<_queue.size()<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (_status->cancel())
    {
        return {};
    }

    //std::cout<<"DatabaseQueue::take_all_when_available() "<<_queue.size()<<std::endl;

    // remove and return the head of the queue
    Nodes nodes;
    nodes.swap(_queue);
    return nodes;
}

/////////////////////////////////////////////////////////////////////////
//
// DatabasePager
//
DatabasePager::DatabasePager()
{
    if (!_status) _status = ActivityStatus::create();

    culledPagedLODs = CulledPagedLODs::create();

    _requestQueue = DatabaseQueue::create(_status);
    _compileQueue = DatabaseQueue::create(_status);
    _toMergeQueue = DatabaseQueue::create(_status);

    pagedLODContainer = PagedLODContainer::create(4000);
}

DatabasePager::~DatabasePager()
{
    //d::cout<<"DatabasePager::~DatabasePager()"<<std::endl;

    _status->set(false);

    for (auto& thread : _readThreads)
    {
        thread.join();
    }

    for (auto& thread : _compileThreads)
    {
        thread.join();
    }
}

void DatabasePager::start()
{
    int numReadThreads = 4;
    int numCompileThreads = 1;

    //
    // set up read thread(s)
    //
    auto read = [](ref_ptr<DatabaseQueue> requestQueue, ref_ptr<DatabaseQueue> compileQueue, ref_ptr<ActivityStatus> status, DatabasePager& databasePager) {
        //std::cout<<"Started DatabaseThread read thread"<<std::endl;

        while (status->active())
        {
            auto plod = requestQueue->take_when_available();
            if (plod)
            {
                uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                if (frameDelta > 1 || !compare_exchange(plod->requestStatus, PagedLOD::ReadRequest, PagedLOD::Reading))
                {
                    // std::cout<<"Expire read request"<<std::endl;
                    databasePager.requestDiscarded(plod);
                    continue;
                }

                //std::cout<<"    reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                auto subgraph = vsg::read_cast<vsg::Node>(plod->filename, plod->options);

                // std::cout<<"    finished reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                if (subgraph && compare_exchange(plod->requestStatus, PagedLOD::Reading, PagedLOD::CompileRequest))
                {
                    {
                        //std::cout<<"   assigned subgraph to plod"<<std::endl;
                        std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                        plod->pending = subgraph;
                    }

                    // move to the merge queue;
                    compileQueue->add_then_reset(plod);
                }
                else
                {
                    databasePager.requestDiscarded(plod);
                }
            }
        }
        //std::cout<<"Finished DatabaseThread read thread"<<std::endl;
    };

    for (int i = 0; i < numReadThreads; ++i)
    {
        _readThreads.emplace_back(read, std::ref(_requestQueue), std::ref(_compileQueue), std::ref(_status), std::ref(*this));
    }

    //
    // set up compile thread(s)
    //
    auto compile = [](ref_ptr<DatabaseQueue> compileQueue, ref_ptr<DatabaseQueue> toMergeQueue, ref_ptr<CompileTraversal> db_ct, ref_ptr<ActivityStatus> status, DatabasePager& databasePager) {
        //std::cout<<"Started DatabaseThread compile thread"<<std::endl;

        std::list<ref_ptr<CompileTraversal>> compileTraversals;

        int numCompileContexts = 16;

        for (int i = 0; i < numCompileContexts; ++i)
        {
            compileTraversals.emplace_back(new CompileTraversal(*db_ct));
        }

        // assign semaphores
        for (auto& ct : compileTraversals)
        {
            ct->context.semaphore = Semaphore::create(ct->context.device, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }

        auto compile_itr = compileTraversals.begin();

        while (status->active())
        {
            auto nodesToCompileOrDelete = compileQueue->take_all_when_available();

            DatabaseQueue::Nodes nodesToCompile;
            for (auto& plod : nodesToCompileOrDelete)
            {
                if (compare_exchange(plod->requestStatus, PagedLOD::DeleteRequest, PagedLOD::Deleting))
                {
#if REPORT_STATS
                    std::cout << "    from compile thread releasing subgraph for plod = " << plod << std::endl;
#endif
                    ref_ptr<Node> subgraph;
                    {
                        std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                        subgraph = plod->pending;
                        plod->pending = nullptr;
                    }

                    //plod->requestStatus.exchange(PagedLOD::NoRequest);
                    databasePager.requestDiscarded(plod);
                }
                else
                {
                    nodesToCompile.emplace_back(plod);
                }
            }

            if (!nodesToCompile.empty())
            {
                CompileTraversal* ct = compile_itr->get();

                ++compile_itr;
                // advance the compile iterator to the next CompileTraversal in the list, wrap around if we get to the end
                if (compile_itr == compileTraversals.end())
                {
                    // std::cout<<"Wrapping around"<<std::endl;
                    compile_itr = compileTraversals.begin();
                }
                else
                {
                    // std::cout<<"Using next CompileTraversal"<<std::endl;
                }

#if REPORT_STATS
                //std::cout<<"Compile Semaphore before wait Semaphore "<<*(ct->context.semaphore->data())<<" , count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;
                int64_t before_wait_memoryTotalAvailable = ct->context.stagingMemoryBufferPools->computeMemoryTotalAvailable();
                int64_t before_wait_memoryTotalReserved = ct->context.stagingMemoryBufferPools->computeMemoryTotalReserved();
                int64_t before_wait_bufferTotalAvailable = ct->context.stagingMemoryBufferPools->computeBufferTotalAvailable();
                int64_t before_wait_bufferTotalReserved = ct->context.stagingMemoryBufferPools->computeBufferTotalReserved();
#endif

#if DO_TIMING
                auto before_wait_for_completion = clock::now();
#endif
                ct->context.waitForCompletion();

#if REPORT_STATS
                int64_t after_wait_memoryTotalAvailable = ct->context.stagingMemoryBufferPools->computeMemoryTotalAvailable();
                int64_t after_wait_memoryTotalReserved = ct->context.stagingMemoryBufferPools->computeMemoryTotalReserved();
                int64_t after_wait_bufferTotalAvailable = ct->context.stagingMemoryBufferPools->computeBufferTotalAvailable();
                int64_t after_wait_bufferTotalReserved = ct->context.stagingMemoryBufferPools->computeBufferTotalReserved();

                std::cout << "waitForComplete() A before_wait_memoryTotalAvailable = " << before_wait_memoryTotalAvailable << ", after_wait_memoryTotalAvailable = " << after_wait_memoryTotalAvailable << std::endl;
                std::cout << "waitForComplete() A before_wait_memoryTotalReserved = " << before_wait_memoryTotalReserved << ", after_wait_memoryTotalReserved = " << after_wait_memoryTotalReserved << std::endl;
                std::cout << "waitForComplete() A before_wait_bufferTotalAvailable = " << before_wait_bufferTotalAvailable << ", after_wait_bufferTotalAvailable = " << after_wait_bufferTotalAvailable << ", delta = " << (after_wait_bufferTotalAvailable - before_wait_bufferTotalAvailable) << std::endl;
                std::cout << "waitForComplete() A before_wait_bufferTotalReserved = " << before_wait_bufferTotalReserved << ", after_wait_bufferTotalReserved = " << after_wait_bufferTotalReserved << ", delta = " << (after_wait_bufferTotalReserved - before_wait_bufferTotalReserved) << std::endl;

                //std::cout<<"Compile Semaphore after wait Semaphore "<<*(ct->context.semaphore->data())<<" , count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;
#endif

                while (ct->context.semaphore->numDependentSubmissions().load() > 0)
                {
                    if (status->cancel()) return;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

#if DO_TIMING
                auto after_wait_and_semaphore = clock::now();
                static double total_wait_time = 0.0;
                static double num_waits = 0.0;
                total_wait_time += std::chrono::duration<double, std::chrono::milliseconds::period>(after_wait_and_semaphore - before_wait_for_completion).count();
                num_waits += 1.0;

                std::cout << "Fence + Semaphore wait : " << std::chrono::duration<double, std::chrono::milliseconds::period>(after_wait_and_semaphore - before_wait_for_completion).count() << " average = " << (total_wait_time / num_waits) << std::endl;
#endif

                ct->context.semaphore->numDependentSubmissions().exchange(1);

                DatabaseQueue::Nodes nodesCompiled;
                for (auto& plod : nodesToCompile)
                {
                    if (compare_exchange(plod->requestStatus, PagedLOD::CompileRequest, PagedLOD::Compiling))
                    {
                        uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                        if (frameDelta <= 1)
                        {
                            // std::cout<<"    compiling "<<plod->filename<<", "<<plod->requestCount.load()<<" Semaphore "<<*(ct->context.semaphore->data())<<", count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;

                            ref_ptr<Node> subgraph;
                            {
                                std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                                subgraph = plod->pending;
                            }

                            // compiling subgraph
                            if (subgraph)
                            {
                                vsg::CollectResourceRequirements collectRequirements;
                                subgraph->accept(collectRequirements);

                                auto maxSets = collectRequirements.requirements.computeNumDescriptorSets();
                                auto descriptorPoolSizes = collectRequirements.requirements.computeDescriptorPoolSizes();

                                // brute force allocation of new DescrptorPool for this subgraph, TODO : need to preallocate large DescritorPoil for multiple loaded subgraphs
                                if (descriptorPoolSizes.size() > 0) ct->context.descriptorPool = vsg::DescriptorPool::create(ct->context.device, maxSets, descriptorPoolSizes);

                                subgraph->accept(*ct);
                                nodesCompiled.emplace_back(plod);
                            }
                            else
                            {
                                // need to reset the PLOD so that it's no longer part of the DatabasePager's queues and is ready to be compile when next requested.
                                std::cout << "Expire compile request " << plod->filename << std::endl;
                                databasePager.requestDiscarded(plod);
                            }
                        }
                        else
                        {
#if 0
                            {
                                std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                                plod->pending = nullptr;
                            }
#endif
#if REPORT_STATS
                            std::cout << "Expire compile request" << std::endl;
#endif
                            // need to reset the PLOD so that it's no longer part of the DatabasePager's queues and is ready to be compile when next requested.
                            databasePager.requestDiscarded(plod);
                        }
                    }
                    else
                    {
                        std::cout << "PagedLOD::requestStatus not DeleteRequest or CompileRequest so ignoring status = " << plod->requestStatus.load() << std::endl;
                    }
                }

#if REPORT_STATS
                int64_t after_complile_traversal_memoryTotalAvailable = ct->context.stagingMemoryBufferPools->computeMemoryTotalAvailable();
                int64_t after_complile_traversal_memoryTotalReserved = ct->context.stagingMemoryBufferPools->computeMemoryTotalReserved();
                int64_t after_complile_traversal_bufferTotalAvailable = ct->context.stagingMemoryBufferPools->computeBufferTotalAvailable();
                int64_t after_complile_traversal_bufferTotalReserved = ct->context.stagingMemoryBufferPools->computeBufferTotalReserved();

                std::cout << "waitForComplete() B after_wait_memoryTotalAvailable = " << after_wait_memoryTotalAvailable << ", after_complile_traversal_memoryTotalAvailable = " << after_complile_traversal_memoryTotalAvailable << std::endl;
                std::cout << "waitForComplete() B after_wait_memoryTotalReserved = " << after_wait_memoryTotalReserved << ", after_complile_traversal_memoryTotalReserved = " << after_complile_traversal_memoryTotalReserved << std::endl;
                std::cout << "waitForComplete() B after_wait_bufferTotalAvailable = " << after_wait_bufferTotalAvailable << ", after_complile_traversal_bufferTotalAvailable = " << after_complile_traversal_bufferTotalAvailable << " delta = " << (after_complile_traversal_bufferTotalAvailable - after_wait_bufferTotalAvailable) << std::endl;
                std::cout << "waitForComplete() B after_wait_bufferTotalReserved = " << after_wait_bufferTotalReserved << ", after_complile_traversal_bufferTotalReserved = " << after_complile_traversal_bufferTotalReserved << " delta = " << (after_complile_traversal_bufferTotalReserved - after_wait_bufferTotalReserved) << std::endl;
#endif
                if (!nodesCompiled.empty())
                {
                    if (ct->context.record())
                    {
                        for (auto& plod : nodesCompiled)
                        {
                            plod->semaphore = ct->context.semaphore;
                            plod->requestStatus.exchange(PagedLOD::MergeRequest);
                        }
                        toMergeQueue->add(nodesCompiled);
                    }
                    else
                    {
                        ct->context.semaphore->numDependentSubmissions().exchange(0);
                        for (auto& plod : nodesCompiled)
                        {
                            databasePager.requestDiscarded(plod);
                        }
                    }
                }
                else
                {
                    ct->context.semaphore->numDependentSubmissions().exchange(0);
                }
            }
        }
        //std::cout<<"Finished DatabaseThread compile thread"<<std::endl;
    };

    for (int i = 0; i < numCompileThreads; ++i)
    {
        _compileThreads.emplace_back(compile, std::ref(_compileQueue), std::ref(_toMergeQueue), std::ref(compileTraversal), std::ref(_status), std::ref(*this));
    }
}

void DatabasePager::request(ref_ptr<PagedLOD> plod)
{
    ++numActiveRequests;

    //std::cout<<"DatabasePager::request("<<plod.get()<<") "<<plod->filename<<", "<<plod->priority<<std::endl;
    bool hasPending = false;
    {
        std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
        hasPending = plod->pending.valid();
    }

    if (hasPending)
    {
        // std::cout<<"DatabasePager::request("<<plod.get()<<") has pending subgraphs to transfer to compile "<<plod->filename<<", "<<plod->priority<<" plod="<<plod.get()<<std::endl;
        if (compare_exchange(plod->requestStatus, PagedLOD::NoRequest, PagedLOD::CompileRequest))
        {
            _compileQueue->add(plod);
        }
        else
        {
            //std::cout<<"Attempted DatabasePager::request("<<plod.get()<<") with pending comile but but plod.requestState() = "<<plod->requestStatus.load()<<" is not NoRequest"<<std::endl;
        }
    }
    else
    {
        if (compare_exchange(plod->requestStatus, PagedLOD::NoRequest, PagedLOD::ReadRequest))
        {
            // std::cout<<"DatabasePager::request("<<plod.get()<<") adding to requeQueue "<<plod->filename<<", "<<plod->priority<<" plod="<<plod.get()<<std::endl;
            _requestQueue->add(plod);
        }
        else
        {
            //std::cout<<"Attempted DatabasePager::request("<<plod.get()<<") but plod.requestState() = "<<plod->requestStatus.load()<<" is not NoRequest"<<std::endl;
        }
    }
}

void DatabasePager::requestDiscarded(PagedLOD* plod)
{
    //std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
    //plod->pending = nullptr;
    plod->requestCount.exchange(0);
    plod->requestStatus.exchange(PagedLOD::NoRequest);
    --numActiveRequests;
}

void DatabasePager::updateSceneGraph(FrameStamp* frameStamp)
{
    frameCount.exchange(frameStamp ? frameStamp->frameCount : 0);

    _semaphores.clear();

    auto nodes = _toMergeQueue->take_all();

    if (culledPagedLODs)
    {
#if DO_TIMING
        auto start_tick = clock::now();
#endif
        auto previous_statusList_count = pagedLODContainer->activeList.count;
        auto& elements = pagedLODContainer->elements;
#if 0
        struct RegisterInActivityStatusPaged : public ConstVisitor
        {
            uint64_t frameCount = 0;
            PagedLODContainer* plodContainer = nullptr;

            RegisterInActivityStatusPaged(uint64_t fc, PagedLODContainer* container) : frameCount(fc), plodContainer(container)  {}

            void apply(const vsg::Node& node) override
            {
                node.traverse(*this);
            }

            void apply(const vsg::PagedLOD& plod) override
            {
                if ((plod.index != 0) && (plodContainer->elements[plod.index].list == &(plodContainer->activeList)) && (plod.requestStatus.load() == PagedLOD::NoRequest) && !plod.highResActive(frameCount))
                {
                    //if (plod.requestStatus.load() != PagedLOD::NoRequest) std::cout<<"RegisterInActivityStatusPaged::apply() requestStatus = "<<plod.requestStatus.load()<<std::endl;

                    //std::cout<<"    nested new inactive "<<&plod<<std::endl;
                    plod.traverse(*this);
                    plodContainer->inactive(&plod);
                }
            }
        } registerInActivityStatusPaged(frameCount, pagedLODContainer);

        // run the registry visitor through the children of the PagedLOD
        for(auto& plod : culledPagedLODs->highresCulled)
        {
            plod->accept(registerInActivityStatusPaged);
        }
#else

        for (auto& plod : culledPagedLODs->highresCulled)
        {
            if ((plod->index != 0) && (elements[plod->index].list == &(pagedLODContainer->activeList)) && !plod->highResActive(frameCount))
            {
                pagedLODContainer->inactive(plod);
            }
        }

        auto& activeList = pagedLODContainer->activeList;
        uint32_t switchedCount = 0;
        for (uint32_t index = activeList.head; index != 0;)
        {
            auto& element = elements[index];
            index = element.next;

            if (!element.plod->highResActive(frameCount))
            {
                //std::cout<<"   active to inactive "<<index<<std::endl;
                ++switchedCount;
                pagedLODContainer->inactive(element.plod.get());
            }
        }

#    if REPORT_STATS
        if (switchedCount > 0)
        {
            std::cout << "active to inactive " << switchedCount << std::endl;
        }
#    endif

#endif

#if DO_TIMING
        auto after_inactive_tick = clock::now();
#endif

        //std::cout<<"  newly active nodes:"<<std::endl;

        for (auto& plod : culledPagedLODs->newHighresRequired)
        {
            pagedLODContainer->active(plod);
        }

        auto after_statusList_count = pagedLODContainer->activeList.count;

        if (after_statusList_count > previous_statusList_count)
        {
            //std::cout<<"previous_statusList_count = "<<previous_statusList_count<<", after_statusList_count = "<<after_statusList_count<<", culledPagedLODs->newHighresRequired = "<<culledPagedLODs->newHighresRequired.size()<<std::endl;
        }

#if DO_TIMING
        auto after_status_tick = clock::now();
#endif

        culledPagedLODs->clear();

        // set the number of PagedLOD to expire
        uint32_t total = pagedLODContainer->activeList.count + pagedLODContainer->inactiveList.count;
        if ((nodes.size() + total) > targetMaxNumPagedLODWithHighResSubgraphs)
        {
            uint32_t numPagedLODHighRestSubgraphsToRemove = (static_cast<uint32_t>(nodes.size()) + total) - targetMaxNumPagedLODWithHighResSubgraphs;
            uint32_t targetNumInactive = (numPagedLODHighRestSubgraphsToRemove < pagedLODContainer->inactiveList.count) ? (pagedLODContainer->inactiveList.count - numPagedLODHighRestSubgraphsToRemove) : 0;

            // std::cout<<"Need to remove, inactive count = "<<pagedLODContainer->inactiveList.count <<", target = "<< targetNumInactive<<std::endl;

            for (uint32_t index = pagedLODContainer->inactiveList.head; (index != 0) && (pagedLODContainer->inactiveList.count > targetNumInactive);)
            {
                auto& element = elements[index];
                index = element.next;

                if (compare_exchange(element.plod->requestStatus, PagedLOD::NoRequest, PagedLOD::DeleteRequest))
                {
                    // std::cout<<"    trimming "<<plod<<std::endl;
                    ref_ptr<PagedLOD> plod = element.plod;
                    plod->children[0].node = nullptr;
                    pagedLODContainer->remove(plod);
                    _compileQueue->add_then_reset(plod);
                }
            }
        }

#if 0

        unsigned int numOrhphanedPagedLOD = 0;
        for(auto& element : pagedLODContainer->elements)
        {
            if (element.plod && element.plod->referenceCount()==1)
            {
                // std::cout<<"plod with reference count 1 also has index = "<<element.plod->index<<" "<<element.list->name<<std::endl;

                ++numOrhphanedPagedLOD;
            }
        }
        // if (numOrhphanedPagedLOD!=0)  std::cout<<"Found PagdLOD in pagedLODContainer without external references "<<numOrhphanedPagedLOD<<std::endl;
#endif

#if DO_TIMING
        auto end_tick = clock::now();
        std::cout << "Time to check for inactive = " << std::chrono::duration<double, std::chrono::milliseconds::period>(after_inactive_tick - start_tick).count() << " active = " << std::chrono::duration<double, std::chrono::milliseconds::period>(after_status_tick - after_inactive_tick).count() << " merge = " << std::chrono::duration<double, std::chrono::milliseconds::period>(end_tick - after_status_tick).count() << "   effective fps = " << (1.0 / std::chrono::duration<double, std::chrono::seconds::period>(end_tick - after_status_tick).count()) << std::endl;
#endif
    }

    if (!nodes.empty())
    {

#define LOCAL_MUTEX 1

#if !LOCAL_MUTEX
        std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
#endif

        //std::cout<<"DatabasePager::updateSceneGraph() nodes to merge : nodes.size() = "<<nodes.size()<<", "<<numActiveRequests.load()<<std::endl;
        for (auto& plod : nodes)
        {
            if (compare_exchange(plod->requestStatus, PagedLOD::MergeRequest, PagedLOD::Merging))
            {
#if DO_TIMING
                if (frameCount.load() > 6)
                {
                    static double totalRequestCount = 0;
                    static double totalNumMerges = 0;

                    totalRequestCount += double(plod->requestCount.load());
                    totalNumMerges += 1.0;

                    std::cout << "average merge count " << (totalRequestCount / totalNumMerges) << std::endl;
                }
#endif
                // std::cout<<"   Merged "<<plod->filename<<" after "<<plod->requestCount.load()<<" priority "<<plod->priority.load()<<" "<<frameCount - plod->frameHighResLastUsed.load()<<" plod = "<<plod.get()<<" "<<*(plod->semaphore->data())<<std::endl;
                {
#if LOCAL_MUTEX
                    std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
#endif
                    plod->children[0].node = plod->pending;
                }

                // insert any semaphore into a set that will be used by the GraphicsStage
                if (plod->semaphore)
                {
                    _semaphores.insert(plod->semaphore);
                    plod->semaphore = nullptr;
                }

                plod->requestStatus.exchange(PagedLOD::NoRequest);
            }
        }
        numActiveRequests -= static_cast<uint32_t>(nodes.size());
    }
    else
    {
        //std::cout<<"DatabasePager::updateSceneGraph() nothing to merge"<<std::endl;
    }
}
