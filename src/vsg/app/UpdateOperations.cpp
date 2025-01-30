/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/UpdateOperations.h>

using namespace vsg;

UpdateOperations::UpdateOperations()
{
}

UpdateOperations::~UpdateOperations()
{
}

void UpdateOperations::add(ref_ptr<Operation> op, RunBehavior runBehavior)
{
    std::scoped_lock<std::mutex> lock(_updateOperationMutex);
    if (runBehavior == ONE_TIME)
        _updateOperationsOneTime.push_back(op);
    else
        _updateOperationsAllFrames.push_back(op);
}

void UpdateOperations::remove(ref_ptr<Operation> op)
{
    std::scoped_lock<std::mutex> lock(_updateOperationMutex);
    _updateOperationsOneTime.remove(op);
    _updateOperationsAllFrames.remove(op);
}

void UpdateOperations::clear()
{
    std::scoped_lock<std::mutex> lock(_updateOperationMutex);
    _updateOperationsOneTime.clear();
    _updateOperationsAllFrames.clear();
}

UpdateOperations::container_type UpdateOperations::getUpdateOperationsOneTime() const
{
    std::scoped_lock<std::mutex> lock(_updateOperationMutex);
    return _updateOperationsOneTime;
}

UpdateOperations::container_type UpdateOperations::getUpdateOperationsAllFrames() const
{
    std::scoped_lock<std::mutex> lock(_updateOperationMutex);
    return _updateOperationsAllFrames;
}

void UpdateOperations::run()
{
    container_type updateOperations;

    {
        std::scoped_lock<std::mutex> lock(_updateOperationMutex);
        _updateOperationsOneTime.swap(updateOperations);
        updateOperations.insert(updateOperations.end(), _updateOperationsAllFrames.begin(), _updateOperationsAllFrames.end());
    }

    for (auto op : updateOperations) op->run();
}
