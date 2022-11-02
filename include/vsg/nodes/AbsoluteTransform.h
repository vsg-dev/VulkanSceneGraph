#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Transform.h>

namespace vsg
{

    /// MatrixTransform is a transform node that provides a 4x4 matrix that is used to position subgraphs in absolute coordinate frame.
    /// During the RecordTraversal the matrix is directly pushed to the State::modelviewMatrixStack stack without the normal multiplication.
    /// After the subgraphs is traversed the matrix is popped from the State::modelviewMatrixStack.
    class VSG_DECLSPEC AbsoluteTransform : public Inherit<Transform, AbsoluteTransform>
    {
    public:
        AbsoluteTransform();
        explicit AbsoluteTransform(const dmat4& in_matrix);

        void read(Input& input) override;
        void write(Output& output) const override;

        dmat4 transform(const dmat4&) const override { return matrix; }

        dmat4 matrix;

    protected:
    };
    VSG_type_name(vsg::AbsoluteTransform);

} // namespace vsg
