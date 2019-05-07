#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

namespace vsg
{

    class MatrixTransform : public Inherit<Group, MatrixTransform>
    {
    public:

#if 0
        using value_type = float;
#else
        using value_type = double;
#endif
        using Matrix = t_mat4<value_type>;
        using MatrixValue = Value<Matrix>;

        MatrixTransform(Allocator* allocator = nullptr);
        MatrixTransform(const Matrix& matrix, Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        void setMatrix(const Matrix& matrix) { (*_matrix) = matrix; }
        Matrix& getMatrix() { return _matrix->value(); }
        const Matrix& getMatrix() const { return _matrix->value(); }

    protected:
        ref_ptr<MatrixValue> _matrix;
    };
    VSG_type_name(vsg::MatrixTransform);

} // namespace vsg
