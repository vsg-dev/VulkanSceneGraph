#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/OperationQueue.h>

#include <list>
#include <mutex>

namespace vsg
{

    /// class for managing thread safe adding and running of update operations
    class VSG_DECLSPEC UpdateOperations : public Inherit<Object, UpdateOperations>
    {
    public:
        UpdateOperations();

        /// specification of whether update operation should be invoked once or on all frames
        enum RunBehavior
        {
            ONE_TIME,
            ALL_FRAMES
        };

        using value_type = ref_ptr<Operation>;
        using container_type = std::list<value_type>;

        /// add updated operation
        virtual void add(ref_ptr<Operation> op, RunBehavior runBehavior = ONE_TIME);

        /// clear all update operations
        void clear();

        /// get a copy of all current one time updated operations
        container_type getUpdateOperationsOneTime() const;

        /// get a copy of all current all frames updated operations
        container_type getUpdateOperationsAllFrames() const;

        /// run is invoked by Viewer::update()
        virtual void run();

    protected:
        virtual ~UpdateOperations();

        mutable std::mutex _updateOperationMutex;
        std::list<ref_ptr<Operation>> _updateOperationsOneTime;
        std::list<ref_ptr<Operation>> _updateOperationsAllFrames;
    };
    VSG_type_name(vsg::UpdateOperations);

} // namespace vsg
