#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/OperationQueue.h>

#include <thread>

namespace vsg
{

    /// OperationThreads provides a collection of std::threads that share a single OperationQueue.
    /// Each thread polls the queue for vsg::Operation to process, when one is available it's removed
    /// from the queue and it's Operation::run() method.
    class VSG_DECLSPEC OperationThreads : public Inherit<Object, OperationThreads>
    {
    public:
        explicit OperationThreads(uint32_t numThreads, ref_ptr<ActivityStatus> in_status = {});
        OperationThreads(const OperationThreads&) = delete;
        OperationThreads& operator=(const OperationThreads& rhs) = delete;

        void add(ref_ptr<Operation> operation)
        {
            queue->add(operation);
        }

        template<typename Iterator>
        void add(Iterator begin, Iterator end)
        {
            queue->add(begin, end);
        }

        /// use this thread to run operations till the queue is empty as well
        /// this thread will consume and run operations in parallel with any threads associated with this OperationThreads.
        void run();

        /// stop threads
        void stop();

        using Threads = std::list<std::thread>;
        Threads threads;
        ref_ptr<OperationQueue> queue;
        ref_ptr<ActivityStatus> status;

    protected:
        virtual ~OperationThreads();
    };
    VSG_type_name(vsg::OperationThreads)

} // namespace vsg
