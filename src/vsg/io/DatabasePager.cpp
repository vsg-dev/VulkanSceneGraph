/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/SharedObjects.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////
//
// DatabaseQueue
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
    // debug("DatabaseQueue::add(", plod,") status = ",plod->requestStatus.load());

    std::scoped_lock lock(_mutex);
    _queue.emplace_back(plod);
    _cv.notify_one();
}

void DatabaseQueue::add(ref_ptr<PagedLOD> plod, const CompileResult& cr)
{
    std::scoped_lock lock(_mutex);
    _queue.emplace_back(plod);
    _cv.notify_one();
    _compileResult.add(cr);
}

ref_ptr<PagedLOD> DatabaseQueue::take_when_available()
{
    // debug("DatabaseQueue::take_when_available() A size = ", _queue.size());

    std::chrono::duration waitDuration = std::chrono::milliseconds(100);
    std::unique_lock lock(_mutex);

    // wait until the conditional variable signals that an operation has been added
    while (_queue.empty() && _status->active())
    {
        // debug("   Waiting on condition variable B size = ", _queue.size());
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer be running go for a quick exit and return nothing.
    if (_queue.empty() || _status->cancel())
    {
        // debug("DatabaseQueue::take_when_available() C empty");
        return {};
    }

    // debug("DatabaseQueue::take_when_available() D ", _queue.size());

    // find the PagedLOD with the highest priority;
    auto itr = _queue.begin();
    auto highest_itr = itr++;

    for (; itr != _queue.end(); ++itr)
    {
        if ((*itr)->priority > (*highest_itr)->priority) highest_itr = itr;
    }

    ref_ptr<PagedLOD> plod = *highest_itr;
    _queue.erase(highest_itr);

    // debug("Returning ", plod.get(), std::dec, ", size = ", _queue.size());
    return plod;
}

DatabaseQueue::Nodes DatabaseQueue::take_all(CompileResult& cr)
{
    std::scoped_lock lock(_mutex);
    Nodes nodes;
    nodes.swap(_queue);
    cr.add(_compileResult);
    _compileResult.reset();
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
    _deleteQueue = DeleteQueue::create(_status);

    pagedLODContainer = PagedLODContainer::create(4000);
}

DatabasePager::~DatabasePager()
{
    debug("DatabasePager::~DatabasePager()");

    stop();
}

void DatabasePager::assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation)
{
    instrumentation = in_instrumentation;
}

void DatabasePager::start(uint32_t numReadThreads)
{
    vsg::debug("DatabasePager::start(", numReadThreads, ")");

    //
    // set up read thread(s)
    //
    auto readThread = [](ref_ptr<DatabaseQueue> requestQueue, ref_ptr<ActivityStatus> status, DatabasePager& databasePager, const std::string& threadName) {
        debug("Started DatabaseThread read thread");

        auto local_instrumentation = shareOrDuplicateForThreadSafety(databasePager.instrumentation);
        if (local_instrumentation) local_instrumentation->setThreadName(threadName);

        while (status->active())
        {
            auto plod = requestQueue->take_when_available();
            if (plod)
            {
                CPU_INSTRUMENTATION_L1_NC(databasePager.instrumentation, "DatabasePager read", COLOR_PAGER);

                uint64_t frameDelta = databasePager.frameCount - plod->frameHighResLastUsed.load();
                if (frameDelta > 1 || !compare_exchange(plod->requestStatus, PagedLOD::ReadRequest, PagedLOD::Reading))
                {
                    // debug("Expire read request");
                    databasePager.requestDiscarded(plod);
                    continue;
                }

                auto read_object = vsg::read(plod->filename, plod->options);
                auto subgraph = read_object.cast<Node>();

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
                        databasePager._toMergeQueue->add(plod, result);
                    }
                    else
                    {
                        debug("Failed to compile ", plod, " ", plod->filename);
                        databasePager.requestDiscarded(plod);
                    }
                }
                else
                {
                    if (auto read_error = read_object.cast<ReadError>())
                        warn(read_error->message);
                    else
                        warn("Failed to read ", plod, " ", plod->filename);

                    databasePager.requestDiscarded(plod);
                }
            }
        }
        debug("Finished DatabaseThread read thread");
    };

    auto deleteThread = [](ref_ptr<DeleteQueue> deleteQueue, ref_ptr<ActivityStatus> status, const DatabasePager& databasePager, const std::string& threadName) {
        debug("Started DatabaseThread deletethread");

        auto local_instrumentation = shareOrDuplicateForThreadSafety(databasePager.instrumentation);
        if (local_instrumentation) local_instrumentation->setThreadName(threadName);

        while (status->active())
        {
            deleteQueue->wait_then_clear();
        }
        debug("Finished DatabaseThread delete thread");
    };

    for (uint32_t i = 0; i < numReadThreads; ++i)
    {
        threads.emplace_back(readThread, std::ref(_requestQueue), std::ref(_status), std::ref(*this), make_string("DatabasePager read thread ", i));
    }

    threads.emplace_back(deleteThread, std::ref(_deleteQueue), std::ref(_status), std::ref(*this), "DatabasePager delete thread ");
}

void DatabasePager::stop()
{
    _status->set(false);

    for (auto& thread : threads)
    {
        thread.join();
    }

    threads.clear();
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
            // debug("DatabasePager::request(", plod.get(), ") adding to requestQueue ", plod->filename, ", ", plod->priority, " plod=", plod.get());
            _requestQueue->add(plod);
        }
        else
        {
            // debug("Attempted DatabasePager::request(", plod.get(), ") but plod.requestState() = ", plod->requestStatus.load(), " is not NoRequest");
        }
    }
    else
    {
        // debug("Attempted DatabasePager::request(", plod.get(), ") but plod.pending is not null.");
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

void DatabasePager::updateSceneGraph(ref_ptr<FrameStamp> frameStamp, CompileResult& cr)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    frameCount.exchange(frameStamp ? frameStamp->frameCount : 0);
    _deleteQueue->advance(frameStamp);

    auto nodes = _toMergeQueue->take_all(cr);

    std::list<ref_ptr<Object>> deleteList;
    std::list<ref_ptr<SharedObjects>> sharedObjectsToPrune;

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
        for (uint32_t index = activeList.head; index != 0;)
        {
            auto& element = elements[index];
            index = element.next;

            if (!element.plod->highResActive(frameCount))
            {
                // debug("   active to inactive ", index);
                pagedLODContainer->inactive(element.plod.get());
            }
        }

        // debug("  newly active nodes:");

        for (auto& plod : culledPagedLODs->newHighresRequired)
        {
            pagedLODContainer->active(plod);
        }

        auto after_statusList_count = pagedLODContainer->activeList.count;

        if (after_statusList_count > previous_statusList_count)
        {
            // debug("previous_statusList_count = ", previous_statusList_count, ", after_statusList_count = ", after_statusList_count, ", culledPagedLODs->newHighresRequired = ", culledPagedLODs->newHighresRequired.size());
        }

        culledPagedLODs->clear();

        // set the number of PagedLOD to expire
        uint32_t total = pagedLODContainer->activeList.count + pagedLODContainer->inactiveList.count;

        debug("DatabasePager : activeList.count = ", pagedLODContainer->activeList.count, ", inactiveList.count = ", pagedLODContainer->inactiveList.count, ", total = ", total);

        if ((nodes.size() + total) > targetMaxNumPagedLODWithHighResSubgraphs)
        {
            uint32_t numPagedLODHighRestSubgraphsToRemove = (static_cast<uint32_t>(nodes.size()) + total) - targetMaxNumPagedLODWithHighResSubgraphs;
            uint32_t targetNumInactive = (numPagedLODHighRestSubgraphsToRemove < pagedLODContainer->inactiveList.count) ? (pagedLODContainer->inactiveList.count - numPagedLODHighRestSubgraphsToRemove) : 0;

            debug("Need to remove, inactive count = ", pagedLODContainer->inactiveList.count, ", target = ", targetNumInactive);

            for (uint32_t index = pagedLODContainer->inactiveList.head; (index != 0) && (pagedLODContainer->inactiveList.count > targetNumInactive);)
            {
                auto& element = elements[index];
                index = element.next;

                if (compare_exchange(element.plod->requestStatus, PagedLOD::NoRequest, PagedLOD::DeleteRequest))
                {
                    ref_ptr<PagedLOD> plod = element.plod;
                    plod->children[0].node = nullptr;
                    plod->requestCount.exchange(0);
                    plod->requestStatus.exchange(PagedLOD::NoRequest);

                    deleteList.push_back(plod->pending);
                    plod->pending = {};

                    deleteList.push_back(plod);
                    pagedLODContainer->remove(plod);

                    if (plod->options->sharedObjects)
                    {
                        if (std::find(sharedObjectsToPrune.begin(), sharedObjectsToPrune.end(), plod->options->sharedObjects) == sharedObjectsToPrune.end())
                        {
                            sharedObjectsToPrune.push_back(plod->options->sharedObjects);
                        }
                    }

                    debug("    trimming ", plod, " ", plod->filename);
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

        debug("DatabasePager::updateSceneGraph() nodes to merge : nodes.size() = ", nodes.size(), ", ", numActiveRequests.load());
        for (auto& plod : nodes)
        {
            if (compare_exchange(plod->requestStatus, PagedLOD::MergeRequest, PagedLOD::Merging))
            {
                debug("   Merged ", plod->filename, " after ", plod->requestCount.load(), " priority ", plod->priority.load(), " ", frameCount - plod->frameHighResLastUsed.load(), " plod = ", plod);
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
        debug("DatabasePager::updateSceneGraph() nothing to merge");
    }

    if (!deleteList.empty() || !sharedObjectsToPrune.empty()) _deleteQueue->add_prune(deleteList, sharedObjectsToPrune);
}
