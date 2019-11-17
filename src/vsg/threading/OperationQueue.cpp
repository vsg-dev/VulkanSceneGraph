/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/threading/OperationQueue.h>

using namespace vsg;

OperationQueue::OperationQueue(ref_ptr<ActivityStatus> status) :
    _status(status)
{
}

ref_ptr<Operation> OperationQueue::take()
{
    std::unique_lock lock(_mutex);

    if (_queue.empty()) return {};

    ref_ptr<Operation> operation = _queue.front();

    _queue.erase(_queue.begin());

    return operation;
}

ref_ptr<Operation> OperationQueue::take_when_available()
{
    std::chrono::duration waitDuration = std::chrono::milliseconds(100);

    std::unique_lock lock(_mutex);

    // wait to the conditional variable signals that an operation has been added
    while (_queue.empty() && _status->active())
    {
        //std::cout<<"Waiting on condition variable"<<std::endl;
        _cv.wait_for(lock, waitDuration);
    }

    // if the threads we are associated with should no longer running go for a quick exit and return nothing.
    if (_status->cancel())
    {
        return {};
    }

    // remove and return the head of the queue
    ref_ptr<Operation> operation = _queue.front();
    _queue.erase(_queue.begin());
    return operation;
}
