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

#define PRINT_CONTAINER 0
#define CHECK_CONTAINER 0

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// PagedLODContainer
//
PagedLODContainer::PagedLODContainer(uint32_t maxNumPagedLOD) :
    elements(1)
{
    availableList.name = "availableList";
    activeList.name = "activeList";
    inactiveList.name = "inactiveList";

    resize(std::max(maxNumPagedLOD, 10u));
}

void PagedLODContainer::resize(uint32_t new_size)
{
    // note first entry in elements is the null entry, so have to add/take away 1 when accounting for it.
    uint32_t original_size = static_cast<uint32_t>(elements.size()) - 1;
    elements.resize(new_size + 1);

    uint32_t i = 1 + original_size;
    uint32_t previous = availableList.tail;

    if (availableList.head == 0)
    {
        availableList.head = i;
    }

    if (availableList.tail > 0)
    {
        elements[availableList.tail].next = i;
    }

    for (; i < new_size; ++i)
    {
        auto& element = elements[i];
        element.previous = previous;
        element.next = i + 1;
        element.list = &availableList;
        previous = i;
    }

    // set up tail
    elements[i].previous = previous;
    elements[i].next = 0;
    elements[i].list = &availableList;

    availableList.tail = i;

    availableList.count += (new_size - original_size);

#if PRINT_CONTAINER
    debug("PagedLODContainer::resize(", new_size, ")");
    debug_stream([&](auto& fout) { print(fout); });
#endif
}

void PagedLODContainer::resize()
{
    uint32_t original_size = static_cast<uint32_t>(elements.size() - 1);
    uint32_t new_size = original_size * 2;
    resize(new_size);
}

void PagedLODContainer::print(std::ostream& fout)
{
    uint32_t total_size = static_cast<uint32_t>(elements.size());
    fout << "    PagedLODContainer::print() elements.size() = " << total_size << std::endl;
    fout << "        availableList, " << &availableList << ", head  = " << availableList.head << ", tail = " << availableList.tail << " count = " << availableList.count << std::endl;
    fout << "        activeList, " << &activeList << ", head  = " << activeList.head << ", tail = " << activeList.tail << " count = " << activeList.count << std::endl;
    fout << "        inactiveList = " << &inactiveList << ", head  = " << inactiveList.head << ", tail = " << inactiveList.tail << " count = " << inactiveList.count << std::endl;

    for (unsigned i = 0; i < total_size; ++i)
    {
        const auto& element = elements[i];
        fout << "         element[" << i << "] plod = " << element.plod.get() << ", previous =" << element.previous << ", next = " << element.next << ", list = ";
        if (element.list)
            fout << element.list->name;
        else
            fout << " unassigned";
        fout << std::endl;
    }
}

void PagedLODContainer::_move(const PagedLOD* plod, List* targetList)
{
    if (plod->index == 0)
    {
#if PRINT_CONTAINER
        debug("plod not yet assigned, assigning to ", targetList->name);
#endif
        // resize if there are no available empty elements.
        if (availableList.head == 0)
        {
            resize();
        }

        // take the first element from availableList and move head to next item.
        uint32_t index = availableList.head;
        auto& element = elements[availableList.head];
        if (availableList.head == availableList.tail)
        {
            availableList.head = 0;
            availableList.tail = 0;
        }
        else
        {
            availableList.head = element.next;
        }

        if (element.next > 0)
        {
            auto& next_element = elements[element.next];
            next_element.previous = 0;
        }

        // place element at the end of the active list.
        if (targetList->tail > 0)
        {
            auto& previous_element = elements[targetList->tail];
            previous_element.next = index;
        }

        if (targetList->head == 0)
        {
            targetList->head = index;
        }

        element.previous = targetList->tail;
        element.next = 0;
        element.list = targetList;
        targetList->tail = index;

        // assign index to PagedLOD
        plod->index = index;
        element.plod = const_cast<PagedLOD*>(plod);

        --(availableList.count);
        ++(targetList->count);

        return;
    }

    auto& element = elements[plod->index];
    List* previousList = element.list;

    if (previousList == targetList)
    {
#if PRINT_CONTAINER
        debug("PagedLODContainer::move(", plod, ") index = ", plod->index, ", already in ", targetList->name);
#endif
        return;
    }

#if PRINT_CONTAINER
    debug("PagedLODContainer::move(", plod, ") index = ", plod->index, ", moving from ", previousList->name, " to ", targetList->name);
#endif

    // remove from inactiveList

    if (element.previous > 0) elements[element.previous].next = element.next;
    if (element.next > 0) elements[element.next].previous = element.previous;

    // if this element is tail on inactive list then shift it back
    if (previousList->head == plod->index)
    {
#if PRINT_CONTAINER
        debug("   removing head from ", previousList->name);
#endif
        previousList->head = element.next;
    }

    if (previousList->tail == plod->index)
    {
#if PRINT_CONTAINER
        debug("   removing tail from ", previousList->name);
#endif
        previousList->tail = element.previous;
    }

    element.list = targetList;
    element.previous = targetList->tail;
    element.next = 0;

    // add to end of activeList tail
    if (targetList->head == 0)
    {
#if PRINT_CONTAINER
        debug("   setting ", targetList->name, ".head to", plod->index);
#endif
        targetList->head = plod->index;
    }

    if (targetList->tail > 0)
    {
#if PRINT_CONTAINER
        debug("   moving ", targetList->name, ".tail to ", plod->index);
#endif
        elements[targetList->tail].next = plod->index;
    }
    targetList->tail = plod->index;

    --(previousList->count);
    ++(targetList->count);
}

void PagedLODContainer::active(const PagedLOD* plod)
{
    debug("Moving to activeList", plod, ", ", plod->index);

    _move(plod, &activeList);

#if PRINT_CONTAINER
    debug_stream([&](auto& fout) { check(); print(fout); });
#endif
}

void PagedLODContainer::inactive(const PagedLOD* plod)
{
    debug("Moving to inactiveList", plod, ", ", plod->index);

    _move(plod, &inactiveList);

#if PRINT_CONTAINER
    debug_stream([&](std::ostream& fout) { check(); print(fout); });
#endif
}

void PagedLODContainer::remove(PagedLOD* plod)
{
    debug("Remove and make available to availableList", plod, ", ", plod->index);

    if (plod->index == 0)
    {
        warn("PagedLODContainer::remove() plod not assigned so ignore");
        check();
        return;
    }

    _move(plod, &availableList);

    // reset element and plod
    auto& element = elements[plod->index];
    plod->index = 0;
    element.plod = nullptr;

#if PRINT_CONTAINER
    check();
#endif
#if CHECK_CONTAINER
    info_stream([&](std::ostream& fout) { print(fout); });
#endif
}

bool PagedLODContainer::check(const List& list)
{
    if (list.head == 0)
    {
        // we have an empty list
        if (list.tail == 0)
        {
            if (list.count == 0) return true;
            warn("list ", list.name, " has a head==0 and tail==0 but length is ", list.count);
            return false;
        }

        warn("list ", list.name, " has a head==0, but tail is non zero");
        return false;
    }

    const auto& head_element = elements[list.head];
    if (head_element.previous != 0)
    {
        warn("list ", list.name, " has a head.previous that is non zero ", head_element.previous);
        return false;
    }

    const auto& tail_element = elements[list.tail];
    if (tail_element.next != 0)
    {
        warn("list ", list.name, " has a tail.next that is non zero ", tail_element.next);
        return false;
    }

    uint32_t count = 0;
    for (uint32_t i = list.head; i > 0 && count < elements.size();)
    {
        auto& element = elements[i];
        if (element.previous == 0)
        {
            if (i != list.head)
            {
                warn("list ", list.name, " non head element ", i, " has a previous==0");
                return false;
            }
        }
        else
        {
            auto& previous_element = elements[element.previous];
            if (previous_element.next != i)
            {
                warn("list ", list.name, " element = ", i, ", element.previous = ", element.previous, ", does not match to previous.next = ", previous_element.next);
                return false;
            }
        }

        if (element.next == 0)
        {
            if (i != list.tail)
            {
                warn("list ", list.name, " non tail element ", i, " has a next==0");
                return false;
            }
        }
        else
        {
            auto& next_element = elements[element.next];
            if (next_element.previous != i)
            {
                warn("list ", list.name, " element = ", i, ", element.next = ", element.next, ", does not match to next.previous = ", next_element.previous);
                return false;
            }
        }

        ++count;

        i = element.next;
    }

    if (count == list.count) return true;
    return false;
}

bool PagedLODContainer::check()
{
    bool result1 = check(availableList);
    bool result2 = check(activeList);
    bool result3 = check(inactiveList);
    return result1 && result2 && result3;
}

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

    _status->set(false);

    for (auto& thread : threads)
    {
        thread.join();
    }
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

                    try
                    {
                        // compile plod
                        if (auto result = databasePager.compileManager->compile(subgraph))
                        {
                            plod->requestStatus.exchange(PagedLOD::MergeRequest);

                            // move to the merge queue;
                            databasePager._toMergeQueue->add(plod, result);
                        }
                        else
                        {
                            databasePager.requestDiscarded(plod);
                        }
                    }
                    catch(...)
                    {
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
