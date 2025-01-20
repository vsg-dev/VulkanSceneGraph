/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/DeleteQueue.h>
#include <vsg/ui/FrameStamp.h>

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

void DeleteQueue::advance(ref_ptr<FrameStamp> frameStamp)
{
    std::scoped_lock lock(_mutex);

    frameCount = frameStamp->frameCount;

    if (!_objectsToDelete.empty() && _objectsToDelete.front().frameCount <= frameStamp->frameCount)
    {
        _cv.notify_one();
    }
}

void DeleteQueue::wait_then_clear()
{
    ObjectsToDelete objectsToDelete;

    {
        std::chrono::duration waitDuration = std::chrono::milliseconds(100);
        std::unique_lock lock(_mutex);

        uint64_t previous_frameCount = frameCount.load();

        // wait until the conditional variable signals that an operation has been added
        while ((_objectsToDelete.empty() || (frameCount.load() == previous_frameCount)) && _status->active())
        {
            _cv.wait_for(lock, waitDuration);
        }
        auto last_itr = std::find_if(_objectsToDelete.begin(), _objectsToDelete.end(), [&](const ObectToDelete& otd) { return otd.frameCount > frameCount; });

        // use a swap of the container to keep the time the mutex is aquired as short as possible
        objectsToDelete.splice(objectsToDelete.end(), _objectsToDelete, _objectsToDelete.begin(), last_itr);
    }

    objectsToDelete.clear();
}

void DeleteQueue::clear()
{
    ObjectsToDelete objectsToDelete;

    // use a swap of the container to keep the time the mutex is aquired as short as possible
    {
        std::scoped_lock lock(_mutex);
        objectsToDelete.swap(objectsToDelete);
    }

    //vsg::info("DeleteQueue::clear(), releasing ", nodesToRelease.size());
    objectsToDelete.clear();
}
