/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/threading/DeleteQueue.h>

using namespace vsg;


/////////////////////////////////////////////////////////////////////////
//
// DeleteQueue
//
DeleteQueue::DeleteQueue(ref_ptr<ActivityStatus> status) :
    _status(status)
{
}

DeleteQueue::~DeleteQueue()
{
}

void DeleteQueue::add(ref_ptr<Node> node)
{
    std::scoped_lock lock(_mutex);
    _nodes.push_back(node);
    _cv.notify_one();
}

void DeleteQueue::add(Nodes& nodes)
{
    std::scoped_lock lock(_mutex);
    _nodes.insert(_nodes.end(), nodes.begin(), nodes.end());
    _cv.notify_one();

    // vsg::info("DeleteQueue::add(Nodes), adding to ", nodes.size(),  ", new has _nodes.size() = ",  _nodes.size());
}

void DeleteQueue::wait_then_clear()
{
    Nodes nodesToRelease;

    {
        std::chrono::duration waitDuration = std::chrono::milliseconds(100);
        std::unique_lock lock(_mutex);

        // wait until the conditional variable signals that an operation has been added
        while (_nodes.empty() && _status->active())
        {
            //info("DeleteQueue::wait_then_clear() Waiting on condition variable, size = ", _nodes.size());
            _cv.wait_for(lock, waitDuration);
        }

        // use a swap of the container to keep the time the mutex is aquired as short as possible
        _nodes.swap(nodesToRelease);
    }

    //vsg::info("DeleteQueue::wait_then_clear(), releasing ", nodesToRelease.size());
    nodesToRelease.clear();
 }

void DeleteQueue::clear()
{
    Nodes nodesToRelease;

    // use a swap of the container to keep the time the mutex is aquired as short as possible
    {
        std::scoped_lock lock(_mutex);
        _nodes.swap(nodesToRelease);
    }

    //vsg::info("DeleteQueue::clear(), releasing ", nodesToRelease.size());
    nodesToRelease.clear();
}
