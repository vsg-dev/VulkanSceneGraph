#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Transform.h>

namespace vsg
{

    /// MatrixTransform is a transform node that provides a 4x4 matrix that is used to position subgraphs.
    /// During the RecordTraversal the matrix is multiplied by the modelview matrix and pushed to the State::modelviewMatrixStack stack.
    /// the subgraphs is traversed the multiplied matrix is popped from the State::modelviewMatrixStack.
    class VSG_DECLSPEC MatrixTransform : public Inherit<Transform, MatrixTransform>
    {
    public:
        MatrixTransform();
        explicit MatrixTransform(const dmat4& in_matrix);

        dmat4 matrix;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        dmat4 transform(const dmat4& mv) const override { return mv * matrix; }

    protected:
    };
    VSG_type_name(vsg::MatrixTransform);

} // namespace vsg
