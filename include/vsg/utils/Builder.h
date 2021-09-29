#pragma once

#include <vsg/all.h>

#define VSG_COMPARE_PARAMETERS(A, B) \
    if (A < B)                       \
        return true;                 \
    else if (B < A)                  \
        return false;

namespace vsg
{
    struct StateInfo
    {
        bool lighting = true;
        bool doubleSided = false;
        bool blending = false;
        bool diffuseMap = false;
        bool wireframe = false;
        bool instancce_colors_vec4 = true;
        bool instancce_positions_vec3 = false;
        ref_ptr<Data> image;
        ref_ptr<Data> displacementMap;

        bool operator<(const StateInfo& rhs) const
        {
            VSG_COMPARE_PARAMETERS(lighting, rhs.lighting)
            VSG_COMPARE_PARAMETERS(doubleSided, rhs.doubleSided)
            VSG_COMPARE_PARAMETERS(blending, rhs.blending)
            VSG_COMPARE_PARAMETERS(diffuseMap, rhs.diffuseMap)
            VSG_COMPARE_PARAMETERS(wireframe, rhs.wireframe)
            VSG_COMPARE_PARAMETERS(instancce_colors_vec4, rhs.instancce_colors_vec4)
            VSG_COMPARE_PARAMETERS(instancce_positions_vec3, rhs.instancce_positions_vec3)
            VSG_COMPARE_PARAMETERS(image, rhs.image)
            return displacementMap < rhs.displacementMap;
        }
    };

    struct GeometryInfo
    {
        vec3 position = {0.0f, 0.0f, 0.0f};
        vec3 dx = {1.0f, 0.0f, 0.0f};
        vec3 dy = {0.0f, 1.0f, 0.0f};
        vec3 dz = {0.0f, 0.0f, 1.0f};
        vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        mat4 transform;

        /// used for instancing
        ref_ptr<vec3Array> positions;
        ref_ptr<Data> colors;

        bool operator<(const GeometryInfo& rhs) const
        {
            VSG_COMPARE_PARAMETERS(position, rhs.position)
            VSG_COMPARE_PARAMETERS(dx, rhs.dx)
            VSG_COMPARE_PARAMETERS(dy, rhs.dy)
            VSG_COMPARE_PARAMETERS(dz, rhs.dz)
            VSG_COMPARE_PARAMETERS(color, rhs.color)
            VSG_COMPARE_PARAMETERS(transform, rhs.transform)
            VSG_COMPARE_PARAMETERS(positions, rhs.positions)
            VSG_COMPARE_PARAMETERS(colors, rhs.colors)
            return false;
        }
    };

    class VSG_DECLSPEC Builder : public Inherit<Object, Builder>
    {
    public:
        bool verbose = false;
        ref_ptr<Options> options;

        /// set up the compile traversal to compile for specified window
        void setup(ref_ptr<Window> window, ViewportState* viewport, uint32_t maxNumTextures = 32);

        void compile(ref_ptr<Node> subgraph);

        ref_ptr<Node> createBox(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCapsule(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCone(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createCylinder(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createDisk(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createQuad(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createSphere(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});
        ref_ptr<Node> createHeightField(const GeometryInfo& info = {}, const StateInfo& stateInfo = {});

    private:
        void transform(const mat4& matrix, ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals);

        uint32_t _allocatedTextureCount = 0;
        uint32_t _maxNumTextures = 0;
        ref_ptr<CompileTraversal> _compile;

        struct DescriptorKey
        {
            ref_ptr<Data> image;
            ref_ptr<Data> displacementMap;

            bool operator<(const DescriptorKey& rhs) const
            {
                VSG_COMPARE_PARAMETERS(image, rhs.image);
                return displacementMap < rhs.displacementMap;
            }
        };

        struct StateSettings
        {
            ref_ptr<DescriptorSetLayout> descriptorSetLayout;
            ref_ptr<PipelineLayout> pipelineLayout;
            ref_ptr<BindGraphicsPipeline> bindGraphicsPipeline;
            std::map<DescriptorKey, ref_ptr<BindDescriptorSets>> textureDescriptorSets;
        };

        std::map<StateInfo, StateSettings> _stateMap;

        StateSettings& _getStateSettings(const StateInfo& stateInfo);
        ref_ptr<BindDescriptorSets> _createDescriptorSet(const StateInfo& stateInfo);

        void _assign(StateGroup& stateGroup, const StateInfo& stateInfo);

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

} // namespace vsg
