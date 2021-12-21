#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/box.h>
#include <vsg/traversals/ArrayState.h>

namespace vsg
{

    class VSG_DECLSPEC ComputeBounds : public Inherit<ConstVisitor, ComputeBounds>
    {
    public:
        ComputeBounds(ref_ptr<ArrayState> initialArrayData = {});

        dbox bounds;

        using ArrayStateStack = std::vector<ref_ptr<ArrayState>>;
        ArrayStateStack arrayStateStack;

        using MatrixStack = std::vector<dmat4>;
        MatrixStack matrixStack;

        void apply(const Object& node) override;
        void apply(const StateGroup& stategroup) override;
        void apply(const Transform& transform) override;
        void apply(const MatrixTransform& transform) override;
        void apply(const Geometry& geometry) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const StateCommand& statecommand) override;

        void apply(uint32_t firstBinding, const DataList& arrays);
        void apply(const vec3Array& vertices) override;
    };
    VSG_type_name(vsg::ComputeBounds);

} // namespace vsg
