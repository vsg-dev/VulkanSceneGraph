/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>

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
    if (!*_active)
    {
        return {};
    }

    // remove and return the head of the queue
    ref_ptr<PagedLOD> plod= _queue.front();
    _queue.erase(_queue.begin());
    return plod;
}

/////////////////////////////////////////////////////////////////////////
//
// DatabasePager
//
DatabasePager::DatabasePager()
{
    if (!_active) _active = Active::create();

    _requestQueue = DatabaseQueue::create(_active);
    _toMergeQueue = DatabaseQueue::create(_active);
}

void DatabasePager::start()
{
    auto run = [](ref_ptr<DatabaseQueue> requestQueue, ref_ptr<DatabaseQueue> mergeQueue, ref_ptr<CompileTraversal> /*ct*/, ref_ptr<Active> a)
    {
        std::cout<<"Started DatabaseThread run"<<std::endl;

        while (*(a))
        {
            auto plod = requestQueue->take_when_avilable();
            if (plod)
            {
                auto subgraph = vsg::read_cast<vsg::Node>(plod->filename);

                if (subgraph)
                {
                    //std::cout<<"   assigned subgraph to plod"<<std::endl;
                    plod->pending = subgraph;

                    // move to the merge queue;
                    mergeQueue->add(plod);
                }
            }
        }
        std::cout<<"Finsihed DatabaseThread"<<std::endl;
    };

    int numThreads = 4;
    for(int i=0; i<numThreads; ++i)
    {
        _databaseThreads.push_back(std::thread(run, std::ref(_requestQueue), std::ref(_toMergeQueue), std::ref(compileTraversal), std::ref(_active)));
    }
}

DatabasePager::~DatabasePager()
{
    std::cout<<"DatabasePager::~DatabasePager()"<<std::endl;

    _active->active = false;

    for(auto& thread : _databaseThreads)
    {
        thread.join();
    }
}

void DatabasePager::request(ref_ptr<PagedLOD> plod)
{
    //std::cout<<"DatabasePager::reqquest("<<plod.get()<<") "<<plod->filename<<", "<<plod->priority<<std::endl;
    _requestQueue->add(plod);
}

void DatabasePager::updateSceneGraph()
{
    auto nodes = _toMergeQueue->take();
    if (!nodes.empty())
    {
        std::cout<<"DatabasePager::updateSceneGraph() nodes to merge"<<std::endl;
        for(auto& plod : nodes)
        {
#if 1
            // compiling subgarph
            if (compileTraversal)
            {
                plod->pending->accept(*compileTraversal);
                compileTraversal->context.dispatchCommands();
            }
#endif
            std::cout<<"   Merged "<<plod->filename<<" after "<<plod->requestCount.load()<<std::endl;
            plod->getChild(0).node = plod->pending;

        }
    }
    else
    {
        std::cout<<"DatabasePager::updateSceneGraph() nothing to merge"<<std::endl;
    }
}
