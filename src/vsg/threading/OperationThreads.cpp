/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/OperationThreads.h>

using namespace vsg;

OperationThreads::OperationThreads(uint32_t numThreads, ref_ptr<ActivityStatus> in_status) :
    status(in_status)
{
    if (!status) status = ActivityStatus::create();
    queue = OperationQueue::create(status);

    auto runThread = [](ref_ptr<OperationQueue> q, ref_ptr<ActivityStatus> thread_status) {
        while (thread_status->active())
        {
            ref_ptr<Operation> operation = q->take_when_available();
            if (operation)
            {
                operation->run();
            }
        }
    };

    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(runThread, queue, status);
    }
}

OperationThreads::~OperationThreads()
{
    stop();
}

void OperationThreads::run()
{
    while (ref_ptr<Operation> operation = queue->take())
    {
        operation->run();
    }
}

void OperationThreads::stop()
{
    status->set(false);

    for (auto& thread : threads)
    {
        thread.join();
    }

    threads.clear();
}
