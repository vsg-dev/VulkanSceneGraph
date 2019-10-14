/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/threading/atomics.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////
//
// DatabasePager
//
DatabaseQueue::DatabaseQueue(ref_ptr<Active> in_active) :
    _active(in_active)
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

void DatabaseQueue::add(Nodes& nodes)
{
    std::scoped_lock lock(_mutex);

    _queue.insert(_queue.end(), nodes.begin(), nodes.end());

    _cv.notify_one();
}


ref_ptr<PagedLOD> DatabaseQueue::take_when_avilable()
{
    //std::cout<<"DatabaseQueue::take_when_avilable() A _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;

    std::chrono::duration waitDuration = std::chrono::milliseconds(100);
    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && *_active)
    {
        //std::cout<<"   Waiting on condition variable B _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (_queue.empty() || !(*_active))
    {
        //std::cout<<"DatabaseQueue::take_when_avilable() C _identifier = "<<_identifier<<" emoty"<<std::endl;
        return {};
    }

    //std::cout<<"DatabaseQueue::take_when_avilable() D _identifier = "<<_identifier<<" "<<_queue.size()<<std::endl;

#if 1

    // find the OagedLOD with the highest priority;
    auto itr = _queue.begin();
    auto highest_itr = itr++;

    for(; itr != _queue.end(); ++itr)
    {
        if ((*highest_itr)->priority > (*itr)->priority) highest_itr = itr;
    }

    ref_ptr<PagedLOD> plod = *highest_itr;
    _queue.erase(highest_itr);

    //std::cout<<"Returning "<<plod.get()<<std::dec<<" variable _identifier = "<<_identifier<<" size = "<<_queue.size()<<std::endl;

#else
    // remove and return the head of the queue
    ref_ptr<PagedLOD> plod= _queue.front();
    _queue.erase(_queue.begin());
#endif
    return plod;
}

DatabaseQueue::Nodes DatabaseQueue::take_all_when_available()
{
    std::chrono::duration waitDuration = std::chrono::milliseconds(100);

    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && *_active)
    {
        //std::cout<<"take_all_when_available() _identifier = "<<_identifier<<" Waiting on condition variable"<<_queue.size()<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }


    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (!*_active)
    {
        return {};
    }

    //std::cout<<"DatabaseQueue::take_all_when_avilable() "<<_queue.size()<<std::endl;

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
    if (!_active) _active = Active::create();

    culledPagedLODs = CulledPagedLODs::create();

    _requestQueue = DatabaseQueue::create(_active);
    _compileQueue = DatabaseQueue::create(_active);
    _toMergeQueue = DatabaseQueue::create(_active);


    pagedLODContainer = PagedLODContainer::create(10000);
}


DatabasePager::~DatabasePager()
{
    //d::cout<<"DatabasePager::~DatabasePager()"<<std::endl;

    _active->active.exchange(false);

    for(auto& thread : _readThreads)
    {
        thread.join();
    }

    for(auto& thread : _compileThreads)
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
    auto read = [](ref_ptr<DatabaseQueue> requestQueue, ref_ptr<DatabaseQueue> compileQueue, ref_ptr<Active> a, DatabasePager& databasePager)
    {
        //std::cout<<"Started DatabaseThread read thread"<<std::endl;

        while (*(a))
        {
            auto plod = requestQueue->take_when_avilable();
            if (plod)
            {
                uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                if (frameDelta>1 || !compare_exchange(plod->requestStatus, PagedLOD::ReadRequest, PagedLOD::Reading))
                {
                    // std::cout<<"Expire read requrest"<<std::endl;
                    databasePager.requestDiscarded(plod);
                    continue;
                }

                //std::cout<<"    reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                auto subgraph = vsg::read_cast<vsg::Node>(plod->filename);

                // std::this_thread::sleep_for(std::chrono::milliseconds(10));

                // std::cout<<"    finished reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                if (subgraph && compare_exchange(plod->requestStatus, PagedLOD::Reading, PagedLOD::CompileRequest))
                {
                    {
                        //std::cout<<"   assigned subgraph to plod"<<std::endl;
                        std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                        plod->pending = subgraph;
                    }

                    // move to the merge queue;
                    compileQueue->add(plod);
                }
                else
                {
                    databasePager.requestDiscarded(plod);
                }
            }
        }
        //std::cout<<"Finsihed DatabaseThread read thread"<<std::endl;
    };

    for(int i=0; i<numReadThreads; ++i)
    {
        _readThreads.emplace_back(std::thread(read, std::ref(_requestQueue), std::ref(_compileQueue), std::ref(_active), std::ref(*this)));
    }

    //
    // set up compile thread(s)
    //
    auto compile = [](ref_ptr<DatabaseQueue> compileQueue, ref_ptr<DatabaseQueue> toMergeQueue, ref_ptr<CompileTraversal> db_ct, ref_ptr<Active> a, DatabasePager& databasePager)
    {
        //std::cout<<"Started DatabaseThread compile thread"<<std::endl;

        std::list<ref_ptr<CompileTraversal>> compileTraversals;

        int numCompileContexts = 1;
#if 1
        for(int i=0; i<numCompileContexts; ++i)
        {
            compileTraversals.emplace_back(new CompileTraversal(*db_ct));
        }
#else
        if (numCompileContexts <= 1)
        {
            compileTraversals.emplace_back(db_ct);
        }
        else
        {
            compileTraversals.emplace_back(db_ct);
            for(int i=1; i<numCompileContexts; ++i)
            {
                compileTraversals.emplace_back(new CompileTraversal(*db_ct));
            }
        }
#endif
        // assign semaphores
        for(auto& ct : compileTraversals)
        {
            ct->context.semaphore = Semaphore::create(ct->context.device);
            ct->context.semaphore->pipelineStageFlags() = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        auto compile_itr = compileTraversals.begin();

        while (*(a))
        {
            auto nodesToCompileOrDelete = compileQueue->take_all_when_available();

            DatabaseQueue::Nodes nodesToCompile;
            for(auto& plod : nodesToCompileOrDelete)
            {
                if (compare_exchange(plod->requestStatus, PagedLOD::DeleteRequest, PagedLOD::Deleting))
                {
                    std::cout<<"    from compile thread releasing subgraph for plod = "<<plod<<std::endl;
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
                // adveance the compile iterator to the next CompileTraversal in the list, wrap around if we get to the end
                if (compile_itr == compileTraversals.end())
                {
                    // std::cout<<"Wrapping around"<<std::endl;
                    compile_itr = compileTraversals.begin();
                }
                else
                {
                    // std::cout<<"Using next CompileTraversal"<<std::endl;
                }

                std::cout<<"Compile Semaphore befoe wait Semaphore "<<*(ct->context.semaphore->data())<<" , count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;

                ct->context.waitForCompletion();

                std::cout<<"Compile Semaphore after wait Semaphore "<<*(ct->context.semaphore->data())<<" , count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;

#if 1
                if (ct->context.semaphore->numDependentSubmissions().load()>0)
                {
                    ct->context.semaphore = Semaphore::create(ct->context.device);
                    ct->context.semaphore->pipelineStageFlags() = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    std::cout<<"Semaphore not ready to reuse so creating a new one "<<*(ct->context.semaphore)<<std::endl;
                }
                else
                {
                    std::cout<<"Reuseing Semaphore "<<*(ct->context.semaphore)<<std::endl;
                }
#else
                auto before_wait_for_semaphore = clock::now();

                while (ct->context.semaphore->numDependentSubmissions().load()>0 && *(a))
                {
                    std::cout<<"Semaphore isn't ready yet Semaphore "<<*(ct->context.semaphore->data())<<", count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                std::cout<<"Semaphore wait : "<<std::chrono::duration<double, std::chrono::milliseconds::period>(clock::now() - before_wait_for_semaphore).count()<<std::endl;


#endif
                ct->context.semaphore->numDependentSubmissions().exchange(1);

                DatabaseQueue::Nodes nodesCompiled;
                for(auto& plod : nodesToCompile)
                {
                    if (compare_exchange(plod->requestStatus, PagedLOD::CompileRequest, PagedLOD::Compiling))
                    {
                        uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                        if (frameDelta<=1)
                        {
                            std::cout<<"    compiling "<<plod->filename<<", "<<plod->requestCount.load()<<" Semaphore "<<*(ct->context.semaphore->data())<<", count "<<ct->context.semaphore->numDependentSubmissions().load()<<std::endl;

                            ref_ptr<Node> subgraph;
                            {
                                std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                                subgraph = plod->pending;
                            }

                            // compiling subgarph
                            if (subgraph)
                            {
                                subgraph->accept(*ct);
                                nodesCompiled.emplace_back(plod);
                            }
                            else
                            {
                                // need to reset the PLOD so that it's no longer part of the DatabasePager's queues and is ready to be compile when next reqested.
                                std::cout<<"Expire compile requrest "<<plod->filename<<std::endl;
                                databasePager.requestDiscarded(plod);
                            }
                        }
                        else
                        {
                            {
                                std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                                plod->pending = nullptr;
                            }

                            // need to reset the PLOD so that it's no longer part of the DatabasePager's queues and is ready to be compile when next reqested.
                            // std::cout<<"Expire compile requrest"<<std::endl;
                            databasePager.requestDiscarded(plod);
                        }
                    }
                    else
                    {
                        std::cout<<"PagedLOD::requestStatus not DeleteRequest or CompileRequest so ignoring status = "<<plod->requestStatus.load()<<std::endl;
                    }
                }

                if (!nodesCompiled.empty())
                {
                    ct->context.dispatch();

                    for(auto& plod : nodesCompiled)
                    {
#if 0
                        if (compare_exchange(plod->requestStatus, PagedLOD::Compiling, PagedLOD::MergeRequest))
                        {
                            plod->semaphore = ct->context.semaphore;

                            //std::cout<<"    finished compile "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;
                            toMergeQueue->add(plod);
                        }
                        else
                        {
                            std::cout<<"PagedLOD::requestStatus not Compiling so ignoring status = "<<plod->requestStatus.load()<<std::endl;
                        }
#else
                            plod->semaphore = ct->context.semaphore;

                            plod->requestStatus.exchange(PagedLOD::MergeRequest);

                            std::cout<<"    finished compile "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;
//                            toMergeQueue->add(plod);
#endif
                    }

                    toMergeQueue->add(nodesCompiled);
                }
            }
        }
        //std::cout<<"Finsihed DatabaseThread compile thread"<<std::endl;
    };

    for(int i=0; i<numCompileThreads; ++i)
    {
        _compileThreads.emplace_back(std::thread(compile, std::ref(_compileQueue), std::ref(_toMergeQueue), std::ref(compileTraversal), std::ref(_active), std::ref(*this)));
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
        // std::cout<<"DatabasePager::reqquest("<<plod.get()<<") has pending subgraphs to transfer to compile "<<plod->filename<<", "<<plod->priority<<" plog="<<plod.get()<<std::endl;
        if (compare_exchange(plod->requestStatus, PagedLOD::NoRequest, PagedLOD::CompileRequest))
        {
            _compileQueue->add(plod);
        }
        else
        {
            std::cout<<"Attempted DatabasePager::reqquest("<<plod.get()<<") with pending comile but but plod.requestState() = "<<plod->requestStatus.load()<<" is not NoRequest"<<std::endl;
        }
    }
    else
    {
        if (compare_exchange(plod->requestStatus, PagedLOD::NoRequest, PagedLOD::ReadRequest))
        {
            // std::cout<<"DatabasePager::reqquest("<<plod.get()<<") adding to requeQueue "<<plod->filename<<", "<<plod->priority<<" plog="<<plod.get()<<std::endl;
            _requestQueue->add(plod);
        }
        else
        {
            //std::cout<<"Attempted DatabasePager::reqquest("<<plod.get()<<") but plod.requestState() = "<<plod->requestStatus.load()<<" is not NoRequest"<<std::endl;
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
        auto start_tick = clock::now();

        auto previous_activeList_count = pagedLODContainer->activeList.count;
        auto& elements = pagedLODContainer->elements;
#if 0
        struct RegisterInActivePaged : public ConstVisitor
        {
            uint64_t frameCount = 0;
            PagedLODContainer* plodContainer = nullptr;

            RegisterInActivePaged(uint64_t fc, PagedLODContainer* container) : frameCount(fc), plodContainer(container)  {}

            void apply(const vsg::Node& node) override
            {
                node.traverse(*this);
            }

            void apply(const vsg::PagedLOD& plod) override
            {
                if ((plod.index != 0) && (plodContainer->elements[plod.index].list == &(plodContainer->activeList)) && (plod.requestStatus.load() == PagedLOD::NoRequest) && !plod.highResActive(frameCount))
                {
                    //if (plod.requestStatus.load() != PagedLOD::NoRequest) std::cout<<"RegisterInActivePaged::apply() requestStatus = "<<plod.requestStatus.load()<<std::endl;

                    //std::cout<<"    nested new inactive "<<&plod<<std::endl;
                    plod.traverse(*this);
                    plodContainer->inactive(&plod);
                }
            }
        } registerInActivePaged(frameCount, pagedLODContainer);

        // run the registry visitor through the children of the PagedLOD
        for(auto& plod : culledPagedLODs->highresCulled)
        {
            plod->accept(registerInActivePaged);
        }
#else

        for(auto& plod : culledPagedLODs->highresCulled)
        {
            if ((plod->index != 0) && (elements[plod->index].list == &(pagedLODContainer->activeList)) && !plod->highResActive(frameCount))
            {
                pagedLODContainer->inactive(plod);
            }
        }

        auto& activeList = pagedLODContainer->activeList;
        uint32_t switchedCount = 0;
        for(uint32_t index = activeList.head; index != 0;)
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

        if (switchedCount>0)
        {
            std::cout<<"active to inactive "<<switchedCount<<std::endl;
        }
#endif
        auto after_inactive_tick = clock::now();

        //std::cout<<"  newly active nodes:"<<std::endl;


        for(auto& plod : culledPagedLODs->newHighresRequired)
        {
            pagedLODContainer->active(plod);
        }

        auto after_activeList_count = pagedLODContainer->activeList.count;

        if (after_activeList_count > previous_activeList_count)
        {
            //std::cout<<"previous_activeList_count = "<<previous_activeList_count<<", after_activeList_count = "<<after_activeList_count<<", culledPagedLODs->newHighresRequired = "<<culledPagedLODs->newHighresRequired.size()<<std::endl;
        }


        auto after_active_tick = clock::now();

        culledPagedLODs->clear();

        // set the number of PagedLOD to expire
        uint32_t total = pagedLODContainer->activeList.count + pagedLODContainer->inactiveList.count;
        if ((nodes.size()+total) > targetMaxNumPagedLODWithHighResSubgraphs)
        {
            uint32_t numPagedLODHighRestSubgraphsToRemove = (nodes.size()+total) - targetMaxNumPagedLODWithHighResSubgraphs;
            uint32_t targetNumInactive = (numPagedLODHighRestSubgraphsToRemove < pagedLODContainer->inactiveList.count) ?
                (pagedLODContainer->inactiveList.count - numPagedLODHighRestSubgraphsToRemove) : 0;

            // std::cout<<"Need to remove, inactive count = "<<pagedLODContainer->inactiveList.count <<", target = "<< targetNumInactive<<std::endl;

            for(uint32_t index = pagedLODContainer->inactiveList.head; (index != 0) && (pagedLODContainer->inactiveList.count > targetNumInactive);)
            {
                auto& element = elements[index];
                index = element.next;

                auto plod = element.plod;
#if 1
                if (compare_exchange(plod->requestStatus, PagedLOD::NoRequest, PagedLOD::DeleteRequest))
#else
                if (plod->requestStatus.exchange(PagedLOD::DeleteRequest))
#endif
                {
                    std::cout<<"    trimming "<<plod<<std::endl;

                    plod->getChild(0).node = nullptr;
                    _compileQueue->add(plod);
                    pagedLODContainer->remove(plod);
                }
            }
        }

#if 1

        unsigned int numOrhphanedPagedLOD = 0;
        for(auto& element : pagedLODContainer->elements)
        {
            if (element.plod && element.plod->referenceCount()==1)
            {
                std::cout<<"plod with reference count 1 also has index = "<<element.plod->index<<" "<<element.list->name<<std::endl;

                ++numOrhphanedPagedLOD;
            }
        }
        if (numOrhphanedPagedLOD!=0)  std::cout<<"Found PagdLOD in pagedLODContainer wihtout external references "<<numOrhphanedPagedLOD<<std::endl;
#endif
        auto end_tick = clock::now();
#if 0
        std::cout<<"Time to check for inactive = "<<std::chrono::duration<double, std::chrono::milliseconds::period>(after_inactive_tick - start_tick).count()<<
                    " active = "<<std::chrono::duration<double, std::chrono::milliseconds::period>(after_active_tick -  after_inactive_tick).count()<<
                    " merge = "<<std::chrono::duration<double, std::chrono::milliseconds::period>(end_tick - after_active_tick).count()<<"   effective fps = "<<(1.0/std::chrono::duration<double, std::chrono::seconds::period>(end_tick - after_active_tick).count())<<std::endl;
#endif

    }

    if (!nodes.empty())
    {
        //std::cout<<"DatabasePager::updateSceneGraph() nodes to merge : nodes.size() = "<<nodes.size()<<", "<<numActiveRequests.load()<<std::endl;
        for(auto& plod : nodes)
        {
#if 1
            plod->requestStatus.exchange(PagedLOD::Merging);
#else
            if (compare_exchange(plod->requestStatus, PagedLOD::MergeRequest, PagedLOD::Merging))
#endif
            {
                std::cout<<"   Merged "<<plod->filename<<" after "<<plod->requestCount.load()<<" priority "<<plod->priority.load()<<" "<<frameCount - plod->frameHighResLastUsed.load()<<" plod = "<<plod.get()<<" "<<*(plod->semaphore->data())<<std::endl;
                {
                    std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
                    plod->getChild(0).node = plod->pending;
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
        numActiveRequests -= nodes.size();
    }
    else
    {
        //std::cout<<"DatabasePager::updateSceneGraph() nothing to merge"<<std::endl;
    }
}
