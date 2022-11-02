#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/transform.h>

#include <stack>

namespace vsg
{
    //** Traverse the scene graph loading any PLOD that are required for a camera view.*/
    class VSG_DECLSPEC LoadPagedLOD : public vsg::Visitor
    {
    public:
        explicit LoadPagedLOD(ref_ptr<Camera> in_camera, int in_loadLevels = 30);

        void apply(Node& node) override;
        void apply(CullNode& node) override;
        void apply(Transform& transform) override;
        void apply(LOD& lod) override;
        void apply(PagedLOD& plod) override;

        ref_ptr<Camera> camera;
        int loadLevels = 0;
        int level = 0;
        unsigned int numTiles = 0;

    protected:
        using Plane = dplane;
        using Polytope = std::array<Plane, 4>;
        using MatrixStack = std::stack<dmat4>;
        using PolytopeStack = std::stack<Polytope>;

        MatrixStack projectionMatrixStack;
        MatrixStack modelviewMatrixStack;

        Polytope _frustumUnit;
        Polytope _frustumProjected;
        PolytopeStack _frustumStack;
        Paths _pathStack;

        inline std::pair<double, double> computeDistanceAndRF(const dsphere& bs) const
        {
            const auto& proj = projectionMatrixStack.top();
            const auto& mv = modelviewMatrixStack.top();
            auto f = -proj[1][1];

            auto dist = std::abs(mv[0][2] * bs.x + mv[1][2] * bs.y + mv[2][2] * bs.z + mv[3][2]);
            auto rf = bs.r * f;
            return {dist, rf};
        }

        void pushFrustum();
    };
    VSG_type_name(vsg::LoadPagedLOD);

} // namespace vsg
