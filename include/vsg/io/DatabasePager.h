#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>

#include <vsg/nodes/PagedLOD.h>

#include <vsg/threading/OperationQueue.h>

#include <vsg/traversals/CompileTraversal.h>

#include <list>
#include <thread>

namespace vsg
{

    class CulledPagedLODs : public Inherit<Object, CulledPagedLODs>
    {
    public:
        CulledPagedLODs()
        {
            highresCulled.reserve(512);
            newHighresRequired.reserve(8);
        }

        void clear()
        {
            highresCulled.clear();
            newHighresRequired.clear();
        }

        std::vector<const PagedLOD*> highresCulled;
        std::vector<const PagedLOD*> newHighresRequired;
    };

    class VSG_DECLSPEC DatabaseQueue : public Inherit<Object, DatabaseQueue>
    {
    public:
        DatabaseQueue(ref_ptr<Active> in_active);

        using Nodes = std::list<ref_ptr<PagedLOD>>;

        Active* getActive() { return _active; }
        const Active* getActive() const { return _active; }

        void add(ref_ptr<PagedLOD> plod);

        // add the plod reference to the queue then set the plod parameter to nullptr to ensure calling thread can't delete it
        void add_then_reset(ref_ptr<PagedLOD>& plod);

        void add(Nodes& nodes);

        ref_ptr<PagedLOD> take_when_avilable();

        Nodes take_all_when_available();

        Nodes take_all()
        {
            std::scoped_lock lock(_mutex);
            Nodes nodes;
            nodes.swap(_queue);
            return nodes;
        }

    protected:
        virtual ~DatabaseQueue();

        std::mutex _mutex;
        std::condition_variable _cv;
        Nodes _queue;
        ref_ptr<Active> _active;
    };
    VSG_type_name(vsg::DatabaseQueue)

        class DatabasePager : public Inherit<Object, DatabasePager>
    {
    public:
        DatabasePager();

        virtual void start();

        virtual void request(ref_ptr<PagedLOD> plod);

        virtual void updateSceneGraph(FrameStamp* frameStamp);

        using Semaphores = std::set<ref_ptr<Semaphore>>;
        Semaphores& getSemaphores() { return _semaphores; }

        ref_ptr<const Options> options;

        ref_ptr<CompileTraversal> compileTraversal;

        std::atomic_uint numActiveRequests = 0;
        std::atomic_uint64_t frameCount;

        ref_ptr<CulledPagedLODs> culledPagedLODs;

        uint32_t targetMaxNumPagedLODWithHighResSubgraphs = 10000;

        std::mutex pendingPagedLODMutex;

        ref_ptr<PagedLODContainer> pagedLODContainer;

    protected:
        virtual ~DatabasePager();

        void requestDiscarded(PagedLOD* plod);

        ref_ptr<Active> _active;

        ref_ptr<DatabaseQueue> _requestQueue;
        ref_ptr<DatabaseQueue> _compileQueue;
        ref_ptr<DatabaseQueue> _toMergeQueue;

        std::list<std::thread> _readThreads;
        std::list<std::thread> _compileThreads;

        Semaphores _semaphores;
    };
    VSG_type_name(vsg::DatabasePager);

} // namespace vsg
