#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/threading/ActivityStatus.h>
#include <vsg/threading/Latch.h>

#include <list>

namespace vsg
{

    /// Template thread safe queue
    template<class T>
    class ThreadSafeQueue : public Inherit<Object, ThreadSafeQueue<T>>
    {
    public:
        using value_type = T;
        using container_type = std::list<value_type>;

        explicit ThreadSafeQueue(ref_ptr<ActivityStatus> status) :
            _status(status)
        {
        }

        ActivityStatus* getStatus() { return _status; }
        const ActivityStatus* getStatus() const { return _status; }

        /// add a single object to the back of the queue
        void add(value_type operation)
        {
            std::scoped_lock lock(_mutex);
            _queue.emplace_back(operation);
            _cv.notify_one();
        }

        /// add multiple objects to the back of the queue
        template<typename Iterator>
        void add(Iterator begin, Iterator end)
        {
            size_t numAdditions = 0;
            std::scoped_lock lock(_mutex);
            for (auto itr = begin; itr != end; ++itr)
            {
                _queue.emplace_back(*itr);
                ++numAdditions;
            }

            if (numAdditions == 1)
                _cv.notify_one();
            else if (numAdditions > 1)
                _cv.notify_all();
        }

        /// return true if the queue is empty.
        bool empty() const
        {
            std::unique_lock lock(_mutex);
            return _queue.empty();
        }

        /// take all available objects from the queue
        container_type take_all()
        {
            std::unique_lock lock(_mutex);

            container_type objects;
            objects.swap(_queue);

            return objects;
        }

        /// take the head from the queue of objects, return null pointer if none are available
        value_type take()
        {
            std::unique_lock lock(_mutex);

            if (_queue.empty()) return {};

            auto operation = _queue.front();

            _queue.erase(_queue.begin());

            return operation;
        }

        /// take the head of the queue, waiting till one is made available if initially empty
        value_type take_when_available()
        {
            std::chrono::duration waitDuration = std::chrono::milliseconds(100);

            std::unique_lock lock(_mutex);

            // wait to the conditional variable signals that an operation has been added
            while (_queue.empty() && _status->active())
            {
                // debug("Waiting on condition variable");
                _cv.wait_for(lock, waitDuration);
            }

            // if the threads we are associated with should no longer running go for a quick exit and return nothing.
            if (_status->cancel())
            {
                return {};
            }

            // remove and return the head of the queue
            auto operation = _queue.front();
            _queue.erase(_queue.begin());
            return operation;
        }

    protected:
        mutable std::mutex _mutex;
        std::condition_variable _cv;
        container_type _queue;
        ref_ptr<ActivityStatus> _status;
    };

    // clang-format screws up handling of VSG_type_name macro so have to switch it off.
    // clang-format off

    /// Operation base class
    struct Operation : public Object
    {
        virtual void run() = 0;
    };
    VSG_type_name(vsg::Operation)

    /// OperationQueue is a thread safe queue of vsg::Operation
    using OperationQueue = ThreadSafeQueue<ref_ptr<Operation>>;
    VSG_type_name(vsg::OperationQueue)

} // namespace vsg
