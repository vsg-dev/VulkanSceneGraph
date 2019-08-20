#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

#include <condition_variable>
#include <list>

namespace vsg
{

struct Active : public Object
{
    Active() : active(true) {}

    std::atomic_bool active;

    explicit operator bool() const noexcept { return active; }

protected:
    virtual ~Active() {}
};


struct Latch : public Object
{
    Latch(uint32_t num) :
        count(num) {}

    void count_down()
    {
        --count;
        if (is_ready())
        {
            std::unique_lock lock(_mutex);
            cv.notify_all();
        }
    }

    bool is_ready() const
    {
        return (count==0);
    }

    void wait()
    {
        // use while loop to return immediate when latch already released
        // and to handle cases where the condition variable releases spuriously.
        while (count != 0)
        {
            std::unique_lock lock(_mutex);
            cv.wait(lock);
        }
    }

    std::atomic_uint count;
    std::mutex _mutex;
    std::condition_variable cv;

protected:
    virtual ~Latch() {}
};

struct Operation : public Object
{
    virtual void run() = 0;
};


class VSG_DECLSPEC OperationQueue : public Inherit<Object, OperationQueue>
{
public:
    OperationQueue(ref_ptr<Active> in_active);

    std::mutex _mutex;
    std::condition_variable _cv;
    std::list<ref_ptr<Operation>> _queue;
    ref_ptr<Active> _active;

    void add(ref_ptr<Operation> operation)
    {
        std::unique_lock lock(_mutex);
        _queue.emplace_back(operation);
        _cv.notify_one();
    }

    template<typename Iterator>
    void add(Iterator begin, Iterator end)
    {
        size_t numAdditions = 0;
        std::unique_lock lock(_mutex);
        for(auto itr = begin; itr != end; ++itr)
        {
            _queue.emplace_back(*itr);
            ++numAdditions;
        }

        if (numAdditions==1) _cv.notify_one();
        else if (numAdditions>1) _cv.notify_all();
    }

    ref_ptr<Operation> take();

    ref_ptr<Operation> take_when_avilable();
};
VSG_type_name(vsg::OperationQueue)

}
