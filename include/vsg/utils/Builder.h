#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/box.h>
#include <vsg/maths/sphere.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>

namespace vsg
{
    struct StateInfo
    {
        bool lighting = true;
        bool doubleSided = false;
        bool blending = false;
        bool greyscale = false; /// greyscale image
        bool wireframe = false;
        bool instance_colors_vec4 = true;
        bool instance_positions_vec3 = false;

        ref_ptr<Data> image;
        ref_ptr<Data> displacementMap;
        ref_ptr<DescriptorSetLayout> viewDescriptorSetLayout;
    };
    VSG_type_name(vsg::StateInfo);

    struct GeometryInfo
    {
        GeometryInfo() = default;

        template<typename T>
        explicit GeometryInfo(const t_box<T>& bb) { set(bb); }

        template<typename T>
        explicit GeometryInfo(const t_sphere<T>& sp) { set(sp); }

        vec3 position = {0.0f, 0.0f, 0.0f};
        vec3 dx = {1.0f, 0.0f, 0.0f};
        vec3 dy = {0.0f, 1.0f, 0.0f};
        vec3 dz = {0.0f, 0.0f, 1.0f};
        vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        mat4 transform;

        template<typename T>
        void set(const t_box<T>& bb)
        {
            position = (bb.min + bb.max) * 0.5f;
            dx.set(bb.max.x - bb.min.x, 0.0f, 0.0f);
            dy.set(0.0f, bb.max.y - bb.min.y, 0.0f);
            dz.set(0.0f, 0.0f, bb.max.z - bb.min.z);
        }

        template<typename T>
        void set(const t_sphere<T>& sp)
        {
            position = sp.center;
            dx.set(sp.radius * 2.0f, 0.0f, 0.0f);
            dy.set(0.0f, sp.radius * 2.0f, 0.0f);
            dz.set(0.0f, 0.0f, sp.radius * 2.0f);
        }

        /// used for instancing
        ref_ptr<vec3Array> positions;
        ref_ptr<Data> colors;

        bool operator<(const GeometryInfo& rhs) const
        {
            int result = compare_region(position, transform, rhs.position);
            if (result) return result < 0;

            if ((result = compare_pointer(positions, rhs.positions))) return result < 0;
            return compare_pointer(colors, rhs.colors) < 0;
        }
    };
    VSG_type_name(vsg::GeometryInfo);

    class VSG_DECLSPEC Builder : public Inherit<Object, Builder>
    {
    public:
        bool verbose = false;
        ref_ptr<Options> options;
        ref_ptr<SharedObjects> sharedObjects;
        ref_ptr<ShaderSet> shaderSet;

        ref_ptr<Node> createBox(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCapsule(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCone(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCylinder(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createDisk(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createQuad(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createSphere(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createHeightField(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});

        ref_ptr<StateGroup> createStateGroup(const StateInfo& stateInfo = {});

        /// assign compile traversal to enable compilation.
        void assignCompileTraversal(ref_ptr<CompileTraversal> ct, uint32_t maxNumTextures = 32);

        ref_ptr<CompileTraversal> compileTraversal;

    private:
        void transform(const mat4& matrix, ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals);

        uint32_t _allocatedTextureCount = 0;
        uint32_t _maxNumTextures = 0;

        vec3 y_texcoord(const StateInfo& info) const;

        using GeometryMap = std::map<GeometryInfo, ref_ptr<Node>>;
        GeometryMap _boxes;
        GeometryMap _capsules;
        GeometryMap _cones;
        GeometryMap _cylinders;
        GeometryMap _quads;
        GeometryMap _spheres;
        GeometryMap _heightfields;

        // used for comparisons
        mat4 identity;
    };
    VSG_type_name(vsg::Builder);

} // namespace vsg
