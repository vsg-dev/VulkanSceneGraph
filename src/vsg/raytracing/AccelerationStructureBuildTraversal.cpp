/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/AccelerationStructureBuildTraversal.h>

using namespace vsg;

AccelerationStructureBuildTraversal::AccelerationStructureBuildTraversal(Device* in_device) :
    Visitor(),
    _device(in_device)
{
    tlas = TopLevelAccelerationStructure::create(_device);
}

void AccelerationStructureBuildTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void AccelerationStructureBuildTraversal::apply(MatrixTransform& mt)
{
    _transformStack.pushAndPreMult(mt.getMatrix());

    mt.traverse(*this);

    _transformStack.pop();
}

void AccelerationStructureBuildTraversal::apply(vsg::Geometry& geometry)
{
    if (geometry.arrays.size() == 0) return;

    // check cache
    auto& blas = _geometryBlasMap[&geometry];
    if (!blas)
    {
        // create new blas and add to cache
        blas = BottomLevelAccelerationStructure::create(_device);
        auto accelGeom = AccelerationGeometry::create();
        accelGeom->verts = geometry.arrays[0];
        accelGeom->indices = geometry.indices;
        blas->geometries.push_back(accelGeom);
    }

    // create a geometry instance for this geometry using the blas that represents it and the current transform matrix
    createGeometryInstance(blas);
}

void AccelerationStructureBuildTraversal::apply(vsg::VertexIndexDraw& vid)
{
    if (vid.arrays.size() == 0) return;

    // check cache
    ref_ptr<BottomLevelAccelerationStructure> blas;
    if (_vertexIndexDrawBlasMap.find(&vid) != _vertexIndexDrawBlasMap.end())
    {
        blas = _vertexIndexDrawBlasMap[&vid];
    }
    else
    {
        // create new blas and add to cache
        blas = BottomLevelAccelerationStructure::create(_device);
        auto accelGeom = AccelerationGeometry::create();
        accelGeom->verts = vid.arrays[0];
        accelGeom->indices = vid.indices;
        blas->geometries.push_back(accelGeom);

        _vertexIndexDrawBlasMap[&vid] = blas;
    }

    // create a geometry instance for this geometry using the blas that represents it and the current transform matrix
    createGeometryInstance(blas);
}

void AccelerationStructureBuildTraversal::createGeometryInstance(BottomLevelAccelerationStructure* blas)
{
    auto geominst = GeometryInstance::create();
    geominst->accelerationStructure = blas;
    geominst->id = static_cast<uint32_t>(tlas->geometryInstances.size());
    geominst->transform = _transformStack.top();

    tlas->geometryInstances.push_back(geominst);
}
