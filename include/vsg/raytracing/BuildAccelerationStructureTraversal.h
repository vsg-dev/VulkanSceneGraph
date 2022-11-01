#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/State.h>

#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>

#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>

#include <vsg/raytracing/AccelerationStructure.h>
#include <vsg/raytracing/BottomLevelAccelerationStructure.h>
#include <vsg/raytracing/TopLevelAccelerationStructure.h>

namespace vsg
{

    /// BuildAccelerationStructureTraversal is helper class for traversal of a scene graph to required acceleration structures.
    class VSG_DECLSPEC BuildAccelerationStructureTraversal : public Visitor
    {
    public:
        explicit BuildAccelerationStructureTraversal(Device* in_device);

        void apply(Object& object) override;
        void apply(Transform& transform) override;
        void apply(Geometry& geometry) override;
        void apply(VertexIndexDraw& vid) override;

        // the top level acceleration structure we are creating and adding geometry instances to as we find and create them
        ref_ptr<TopLevelAccelerationStructure> tlas;

    protected:
        void createGeometryInstance(BottomLevelAccelerationStructure* blas);

        ref_ptr<Device> _device;

        MatrixStack _transformStack;

        // cache blas's created for various types of draw node
        std::map<VertexIndexDraw*, ref_ptr<BottomLevelAccelerationStructure>> _vertexIndexDrawBlasMap;
        std::map<Geometry*, ref_ptr<BottomLevelAccelerationStructure>> _geometryBlasMap;
    };
    VSG_type_name(vsg::BuildAccelerationStructureTraversal);

} // namespace vsg
