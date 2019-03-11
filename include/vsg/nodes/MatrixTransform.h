#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/vk/PushConstants.h>

namespace vsg
{

    class MatrixTransform : public Inherit<Group, MatrixTransform>
    {
    public:
        MatrixTransform(Allocator* allocator = nullptr);
        MatrixTransform(const mat4& matrix, Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        void setMatrix(const mat4& matrix) { (*_matrix) = matrix; }
        mat4& getMatrix() { return _matrix->value(); }
        const mat4& getMatrix() const { return _matrix->value(); }

        inline void pushTo(State& state) const
        {
            _pushConstant->pushTo(state);
        }

        inline void popFrom(State& state) const
        {
            _pushConstant->popFrom(state);
        }

    protected:
        ref_ptr<mat4Value> _matrix;
        ref_ptr<PushConstants> _pushConstant;
    };
    VSG_type_name(vsg::MatrixTransform);

}
