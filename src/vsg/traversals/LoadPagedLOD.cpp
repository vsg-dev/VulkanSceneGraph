/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/FileSystem.h>
#include <vsg/io/read.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/traversals/LoadPagedLOD.h>

using namespace vsg;

LoadPagedLOD::LoadPagedLOD(ref_ptr<Camera> in_camera, int in_loadLevels) :
    camera(in_camera),
    loadLevels(in_loadLevels)
{
    projectionMatrixStack.emplace(camera->projectionMatrix->transform());
    modelviewMatrixStack.emplace(camera->viewMatrix->transform());

    _frustumUnit = Polytope{{
        Plane(1.0, 0.0, 0.0, 1.0),  // left plane
        Plane(-1.0, 0.0, 0.0, 1.0), // right plane
        Plane(0.0, 1.0, 0.0, 1.0),  // bottom plane
        Plane(0.0, -1.0, 0.0, 1.0)  // top plane
    }};

    const auto& proj = projectionMatrixStack.top();

    _frustumProjected[0] = _frustumUnit[0] * proj;
    _frustumProjected[1] = _frustumUnit[1] * proj;
    _frustumProjected[2] = _frustumUnit[2] * proj;
    _frustumProjected[3] = _frustumUnit[3] * proj;

    pushFrustum();
}

void LoadPagedLOD::pushFrustum()
{
    const auto mv = modelviewMatrixStack.top();
    _frustumStack.push(Polytope{{_frustumProjected[0] * mv,
                                 _frustumProjected[1] * mv,
                                 _frustumProjected[2] * mv,
                                 _frustumProjected[3] * mv}});
}

void LoadPagedLOD::apply(Node& node)
{
    node.traverse(*this);
}

void LoadPagedLOD::apply(CullNode& node)
{
    // check if cullNode bounding sphere is in view frustum.
    if (!intersect(_frustumStack.top(), node.bound)) return;

    //std::cout<<"apply(CullNode& node) : Need to do cull test of bounding sphere"<<std::endl;
    node.traverse(*this);
}

void LoadPagedLOD::apply(Transform& transform)
{
    //std::cout<<"apply(Transform& transform) Need to do transform modelview matrix"<<std::endl;

    modelviewMatrixStack.emplace(transform.transform(modelviewMatrixStack.top()));

    pushFrustum();

    transform.traverse(*this);

    _frustumStack.pop();

    modelviewMatrixStack.pop();
}

void LoadPagedLOD::apply(LOD& lod)
{
    auto bs = lod.bound;

    // check if lod bounding sphere is in view frustum.
    if (!intersect(_frustumStack.top(), bs)) return;

    auto [distance, rf] = computeDistanceAndRF(bs);

    for (auto& child : lod.children)
    {
        bool child_visible = rf > (child.minimumScreenHeightRatio * distance);
        if (child_visible)
        {
            child.node->accept(*this);
            return;
        }
    }
}

void LoadPagedLOD::apply(PagedLOD& plod)
{
    auto bs = plod.bound;

    // check if lod bounding sphere is in view frustum.
    if (level >= loadLevels || !intersect(_frustumStack.top(), bs)) return;

    //std::cout<<"PLOD intersects "<< (intersect(_frustumStack.top(), bs))<<std::endl;

    auto [distance, rf] = computeDistanceAndRF(bs);

    for (auto& child : plod.children)
    {
        bool child_visible = rf > child.minimumScreenHeightRatio * distance;
        if (child_visible)
        {
            ++level;

            Path filename = _pathStack.empty() ? plod.filename : concatPaths(_pathStack.back(), plod.filename);

            Path localPath = filePath(plod.filename);
            if (!localPath.empty())
            {
                if (_pathStack.empty())
                    _pathStack.push_back(localPath);
                else
                    _pathStack.push_back(concatPaths(_pathStack.back(), localPath));
            }

            if (!child.node)
            {
                child.node = read_cast<Node>(filename, plod.options);
                ++numTiles;
            }

            if (child.node) child.node->accept(*this);

            if (!localPath.empty())
            {
                _pathStack.pop_back();
            }

            --level;
        }
    }
}
