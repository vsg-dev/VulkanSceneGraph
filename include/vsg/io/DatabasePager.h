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

#include <vsg/threading/ActivityStatus.h>

#include <vsg/app/CompileManager.h>

#include <condition_variable>
#include <list>
#include <thread>

namespace vsg
{

    /// Class used by the DatabasePager to keep track of PagedLOD nodes
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

    /// Thread safe queue for tracking PagedLOD that needs to be loaded, compiled or merged by the DatabasePager
    class VSG_DECLSPEC DatabaseQueue : public Inherit<Object, DatabaseQueue>
    {
    public:
        explicit DatabaseQueue(ref_ptr<ActivityStatus> status);

        using Nodes = std::list<ref_ptr<PagedLOD>>;

        ActivityStatus* getStatus() { return _status; }
        const ActivityStatus* getStatus() const { return _status; }

        void add(ref_ptr<PagedLOD> plod);

        void add(ref_ptr<PagedLOD> plod, const CompileResult& cr);

        ref_ptr<PagedLOD> take_when_available();

        Nodes take_all(CompileResult& result);

    protected:
        virtual ~DatabaseQueue();

        std::mutex _mutex;
        std::condition_variable _cv;
        Nodes _queue;
        CompileResult _compileResult;
        ref_ptr<ActivityStatus> _status;
    };
    VSG_type_name(vsg::DatabaseQueue);

    /// Multi-threaded database pager for reading, compiling loaded PagedLOD subgraphs and updating the scene graph
    /// with newly loaded subgraphs and pruning expired PageLOD subgraphs
    class VSG_DECLSPEC DatabasePager : public Inherit<Object, DatabasePager>
    {
    public:
        DatabasePager();

        DatabasePager(const DatabasePager&) = delete;
        DatabasePager& operator=(const DatabasePager& rhs) = delete;

        virtual void start();

        virtual void request(ref_ptr<PagedLOD> plod);

        virtual void updateSceneGraph(FrameStamp* frameStamp, CompileResult& cr);

        ref_ptr<const Options> options;

        ref_ptr<CompileManager> compileManager;

        std::atomic_uint numActiveRequests{0};
        std::atomic_uint64_t frameCount;

        ref_ptr<CulledPagedLODs> culledPagedLODs;

        /// for systems for smaller GPU memory limits you may need to reduce the targetMaxNumPagedLODWithHighResSubgraphs to keep memory usage within available limits.
        uint32_t targetMaxNumPagedLODWithHighResSubgraphs = 1500;

        std::mutex pendingPagedLODMutex;

        ref_ptr<PagedLODContainer> pagedLODContainer;

    protected:
        virtual ~DatabasePager();

        void requestDiscarded(PagedLOD* plod);

        ref_ptr<ActivityStatus> _status;

        ref_ptr<DatabaseQueue> _requestQueue;
        ref_ptr<DatabaseQueue> _toMergeQueue;

        std::list<std::thread> _readThreads;
    };
    VSG_type_name(vsg::DatabasePager);

} // namespace vsg
