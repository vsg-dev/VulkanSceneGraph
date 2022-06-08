/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>
#include <vsg/io/stream.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>

#include <iostream>

using namespace vsg;

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
}

void DatabasePager::start()
{
    int numReadThreads = 4;

    //
    // set up read thread(s)
    //
    auto read = [](ref_ptr<DatabaseQueue> requestQueue, ref_ptr<ActivityStatus> status, DatabasePager& databasePager) {
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

                auto subgraph = vsg::read_cast<vsg::Node>(plod->filename, plod->options);

                if (subgraph && compare_exchange(plod->requestStatus, PagedLOD::Reading, PagedLOD::Compiling))
                {
                    {
                        std::scoped_lock<std::mutex> lock(databasePager.pendingPagedLODMutex);
                        plod->pending = subgraph;
                    }

                    // compile plod
                    if (auto result = databasePager.compileManager->compile(subgraph))
                    {
                        plod->requestStatus.exchange(PagedLOD::MergeRequest);

                        // move to the merge queue;
                        databasePager._toMergeQueue->add(plod);
                    }
                    else
                    {
                        // std::cout<<"Failed to compile "<<plod<<" "<<plod->filename<<std::endl;
                        databasePager.requestDiscarded(plod);
                    }
                }
                else
                {
                    // std::cout<<"Failed to read "<<plod<<" "<<plod->filename<<std::endl;
                    databasePager.requestDiscarded(plod);
                }
            }
        }
        //std::cout<<"Finished DatabaseThread read thread"<<std::endl;
    };

    for (int i = 0; i < numReadThreads; ++i)
    {
        _readThreads.emplace_back(read, std::ref(_requestQueue), std::ref(_status), std::ref(*this));
    }
}

void DatabasePager::request(ref_ptr<PagedLOD> plod)
{
    ++numActiveRequests;

    bool hasPending = false;
    {
        std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
        hasPending = plod->pending.valid();
    }

    if (!hasPending)
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
    else
    {
        //std::cout<<"Attempted DatabasePager::request("<<plod.get()<<") but plod.pending is not null."<<std::endl;
    }
}

void DatabasePager::requestDiscarded(PagedLOD* plod)
{
    //std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
    //plod->pending = nullptr;
    plod->requestCount.exchange(0);
    plod->requestStatus.exchange(PagedLOD::NoRequest);
    plod->pending = {};
    --numActiveRequests;
}

void DatabasePager::updateSceneGraph(FrameStamp* frameStamp)
{
    frameCount.exchange(frameStamp ? frameStamp->frameCount : 0);

    auto nodes = _toMergeQueue->take_all();

    if (culledPagedLODs)
    {
        auto previous_statusList_count = pagedLODContainer->activeList.count;
        auto& elements = pagedLODContainer->elements;

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

        culledPagedLODs->clear();

        // set the number of PagedLOD to expire
        uint32_t total = pagedLODContainer->activeList.count + pagedLODContainer->inactiveList.count;

        // std::cout<<"DatabasePager : activeList.count = "<<pagedLODContainer->activeList.count<<", inactiveList.count = "<<pagedLODContainer->inactiveList.count<<", total = "<<total<<std::endl;

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
                    ref_ptr<PagedLOD> plod = element.plod;
                    // std::cout<<"    trimming "<<plod<<" "<<plod->filename<<std::endl;
                    plod->children[0].node = nullptr;
                    plod->requestCount.exchange(0);
                    plod->requestStatus.exchange(PagedLOD::NoRequest);
                    plod->pending = {};
                    pagedLODContainer->remove(plod);
                }
            }
        }
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
                // std::cout<<"   Merged "<<plod->filename<<" after "<<plod->requestCount.load()<<" priority "<<plod->priority.load()<<" "<<frameCount - plod->frameHighResLastUsed.load()<<" plod = "<<plod.get()<<std::endl;
                {
#if LOCAL_MUTEX
                    std::scoped_lock<std::mutex> lock(pendingPagedLODMutex);
#endif
                    plod->children[0].node = plod->pending;
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
