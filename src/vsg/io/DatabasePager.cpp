/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>
#include <vsg/ui/ApplicationEvent.h>

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

ref_ptr<PagedLOD> DatabaseQueue::take_when_avilable()
{
    std::chrono::duration waitDuration = std::chrono::milliseconds(100);
    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && *_active)
    {
        //std::cout<<"Waiting on condition variable"<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (_queue.empty() || !(*_active))
    {
        return {};
    }

    //std::cout<<"DatabaseQueue::take_when_avilable() "<<_queue.size()<<std::endl;

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

    return plod;

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
        //std::cout<<"Waiting on condition variable"<<std::endl;
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

    activePagedLODs = PagedLODList::create();
    inactivePagedLODs = PagedLODList::create();

    culledPagedLODs = CulledPagedLODs::create();

    _requestQueue = DatabaseQueue::create(_active);
    _compileQueue = DatabaseQueue::create(_active);
    _toMergeQueue = DatabaseQueue::create(_active);
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
                if (frameDelta>1)
                {
                    // std::cout<<"Expire read requrest"<<std::endl;
                    databasePager.requestDiscarded(plod);
                    continue;
                }

                // std::cout<<"    reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                auto subgraph = vsg::read_cast<vsg::Node>(plod->filename);

                // std::this_thread::sleep_for(std::chrono::milliseconds(10));

                //std::cout<<"    finished reading "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                if (subgraph)
                {
                    //std::cout<<"   assigned subgraph to plod"<<std::endl;
                    plod->pending = subgraph;

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

        int numCompileContexts = 30;
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
        }

        auto compile_itr = compileTraversals.begin();

        while (*(a))
        {
            auto nodesToCompile = compileQueue->take_all_when_available();

            if (!nodesToCompile.empty())
            {
                CompileTraversal* ct = compile_itr->get();

                ++compile_itr;
                // adveance the compile iterator to the next CompileTraversal in the list, wrap around if we get to the end
                if (compile_itr == compileTraversals.end())
                {
                    // std::cout<<"Wrapping aroind"<<std::endl;
                    compile_itr = compileTraversals.begin();
                }
                else
                {
                    // std::cout<<"Using next CompileTraversal"<<std::endl;
                }


                ct->context.waitForCompletion();

                DatabaseQueue::Nodes nodesCompiled;
                for(auto& plod : nodesToCompile)
                {
                    uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                    if (frameDelta<=1)
                    {
                        // std::cout<<"    compiling "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;

                        // compiling subgarph
                        plod->pending->accept(*ct);

                        nodesCompiled.emplace_back(plod);
                    }
                    else
                    {
                        // need to reset the PLOD so that it's no longer part of the DatabasePager's queues and is ready to be compile when next reqested.
                        // std::cout<<"Expire compile requrest"<<std::endl;
                        databasePager.requestDiscarded(plod);
                    }
                }

                if (!nodesCompiled.empty())
                {
                    ct->context.dispatch();

                    for(auto& plod : nodesCompiled)
                    {
                        plod->semaphore = ct->context.semaphore;

                        //std::cout<<"    finished compile "<<plod->filename<<", "<<plod->requestCount.load()<<std::endl;
                        toMergeQueue->add(plod);
                    }
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

    //std::cout<<"DatabasePager::reqquest("<<plod.get()<<") "<<plod->filename<<", "<<plod->priority<<std::endl;
    if (plod->pending)
    {
        //std::cout<<"DatabasePager::reqquest("<<plod.get()<<") has pending subgraphs to transfer to compile "<<plod->filename<<", "<<plod->priority<<" plog="<<plod.get()<<std::endl;
        _compileQueue->add(plod);
    }
    else
    {
        _requestQueue->add(plod);
    }
}

void DatabasePager::updateSceneGraph(FrameStamp* frameStamp)
{
    frameCount.exchange(frameStamp ? frameStamp->frameCount : 0);

    _semaphores.clear();

    auto nodes = _toMergeQueue->take_all();

    if (culledPagedLODs)
    {
        struct RegisterInActivePaged : public ConstVisitor
        {
            uint64_t frameCount = 0;
            PagedLODList* inactiveList = nullptr;

            RegisterInActivePaged(uint64_t fc, PagedLODList* inactive) : frameCount(fc), inactiveList(inactive)  {}

            void apply(const vsg::Node& node) override
            {
                node.traverse(*this);
            }

            void apply(const vsg::PagedLOD& plod) override
            {
                if (plod.list && plod.list != inactiveList && !plod.highResActive(frameCount))
                {
                    //std::cout<<"    nested new inactive "<<std::endl;
                    plod.traverse(*this);
                    inactiveList->add(const_cast<PagedLOD*>(&plod));
                }
            }
        } registerInActivePaged(frameCount, inactivePagedLODs);

        // run the registry visitor through the children of the PagedLOD
        for(auto& plod : culledPagedLODs->highresCulled)
        {
            plod->accept(registerInActivePaged);
        }

        //std::cout<<"  newly active nodes:"<<std::endl;
        for(auto& plod : culledPagedLODs->newHighresRequired)
        {
            activePagedLODs->add(const_cast<PagedLOD*>(plod));
        }

        culledPagedLODs->clear();

        // set the number of PageDLOD to expire
        uint32_t total = inactivePagedLODs->count + activePagedLODs->count;
        if ((nodes.size()+total) > targetMaxNumPagedLODWithHighResSubgraphs)
        {
            uint32_t numPagedLODHighRestSubgraphsToRemove = (nodes.size()+total) - targetMaxNumPagedLODWithHighResSubgraphs;
            uint32_t targetNumInactive = (numPagedLODHighRestSubgraphsToRemove < inactivePagedLODs->count) ?
                (inactivePagedLODs->count - numPagedLODHighRestSubgraphsToRemove) : 0;

            while(inactivePagedLODs->count > targetNumInactive)
            {
                PagedLOD* plod = inactivePagedLODs->head;
                if (plod)
                {
                    // std::cout<<"    trimming "<<plod<<std::endl;
                    inactivePagedLODs->remove(plod);
                    plod->getChild(0).node = nullptr;
                    plod->pending = nullptr;
                    plod->requestCount.exchange(0);
                    plod->frameHighResLastUsed.exchange(0);
                }
            }
        }

    }

    if (!nodes.empty())
    {
        //std::cout<<"DatabasePager::updateSceneGraph() nodes to merge : nodes.size() = "<<nodes.size()<<", "<<numActiveRequests.load()<<std::endl;
        for(auto& plod : nodes)
        {
            //std::cout<<"   Merged "<<plod->filename<<" after "<<plod->requestCount.load()<<" priority "<<plod->priority.load()<<" "<<frameCount - plod->frameHighResLastUsed.load()<<std::endl;
            plod->getChild(0).node = plod->pending;

            // insert any semaphore into a set that will be used by the GraphicsStage
            if (plod->semaphore) _semaphores.insert(plod->semaphore);

        }
        numActiveRequests -= nodes.size();
    }
    else
    {
        //std::cout<<"DatabasePager::updateSceneGraph() nothing to merge"<<std::endl;
    }
}
