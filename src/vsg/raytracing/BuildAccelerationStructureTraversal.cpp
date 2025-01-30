/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/BuildAccelerationStructureTraversal.h>

using namespace vsg;

BuildAccelerationStructureTraversal::BuildAccelerationStructureTraversal(Device* in_device) :
    Visitor(),
    _device(in_device)
{
    tlas = TopLevelAccelerationStructure::create(_device);
}

void BuildAccelerationStructureTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void BuildAccelerationStructureTraversal::apply(Transform& transform)
{
    _transformStack.push(transform);

    transform.traverse(*this);

    _transformStack.pop();
}

void BuildAccelerationStructureTraversal::apply(Geometry& geometry)
{
    if (geometry.arrays.empty()) return;

    // check cache
    auto& blas = _geometryBlasMap[&geometry];
    if (!blas)
    {
        // create new blas and add to cache
        blas = BottomLevelAccelerationStructure::create(_device);
#if 0 // TODO
        auto accelGeom = AccelerationGeometry::create();
        accelGeom->verts = geometry.arrays[0];
        accelGeom->indices = geometry.indices;
        blas->geometries.push_back(accelGeom);
#else
        throw "BuildAccelerationStructureTraversal::compile() not implemented";
#endif
    }

    // create a geometry instance for this geometry using the blas that represents it and the current transform matrix
    createGeometryInstance(blas);
}

void BuildAccelerationStructureTraversal::apply(VertexIndexDraw& vid)
{
    if (vid.arrays.empty()) return;

    // check cache
    auto& blas = _vertexIndexDrawBlasMap[&vid];
    if (!blas)
    {
        blas = BottomLevelAccelerationStructure::create(_device);
        auto accelGeom = AccelerationGeometry::create();
        accelGeom->assignVertices(vid.arrays[0]->data);
        accelGeom->assignIndices(vid.indices->data);
        blas->geometries.push_back(accelGeom);
    }

    // create a geometry instance for this geometry using the blas that represents it and the current transform matrix
    createGeometryInstance(blas);
}

void BuildAccelerationStructureTraversal::createGeometryInstance(BottomLevelAccelerationStructure* blas)
{
    auto geominst = GeometryInstance::create();
    geominst->accelerationStructure = blas;
    geominst->id = static_cast<uint32_t>(tlas->geometryInstances.size());
    geominst->transform = _transformStack.top();

    tlas->geometryInstances.push_back(geominst);
}
