#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/ActivityStatus.h>

#include <condition_variable>
#include <list>

namespace vsg
{

    class VSG_DECLSPEC DeleteQueue : public Inherit<Object, DeleteQueue>
    {
    public:
        explicit DeleteQueue(ref_ptr<ActivityStatus> status);

        using Nodes = std::list<ref_ptr<Node>>;

        ActivityStatus* getStatus() { return _status; }
        const ActivityStatus* getStatus() const { return _status; }

        void add(ref_ptr<Node> node);

        void add(Nodes& nodes);

        void wait_then_clear();

        void clear();

    protected:
        virtual ~DeleteQueue();

        std::mutex _mutex;
        std::condition_variable _cv;
        Nodes _nodes;
        ref_ptr<ActivityStatus> _status;
    };
    VSG_type_name(vsg::DeleteQueue);

} // namespace vsg
