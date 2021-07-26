#pragma once

#include <vsg/all.h>

namespace vsg
{

    struct GeometryInfo
    {
        vec3 position = {0.0, 0.0, 0.0};
        vec3 dx = {1.0f, 0.0f, 0.0f};
        vec3 dy = {0.0f, 1.0f, 0.0f};
        vec3 dz = {0.0f, 0.0f, 1.0f};
        vec4 color = {1.0, 1.0, 1.0, 1.0};
        ref_ptr<Data> image;

        bool operator<(const GeometryInfo& rhs) const
        {
            if (position < rhs.position) return true;
            if (rhs.position < position) return false;

            if (dx < rhs.dx) return true;
            if (rhs.dx < dx) return false;

            if (dy < rhs.dy) return true;
            if (rhs.dy < dy) return false;

            if (dz < rhs.dz) return true;
            if (rhs.dz < dz) return false;

            if (color < rhs.color) return true;
            if (rhs.color < color) return false;

            return image < rhs.image;
        }
    };

    class Builder : public Inherit<Object, Builder>
    {
    public:
        bool verbose = false;

        /// set up the compile traversal to compile for specified window
        void setup(ref_ptr<Window> window, ViewportState* viewport, uint32_t maxNumTextures = 32);

        void compile(ref_ptr<Node> subgraph);

        ref_ptr<Node> createBox(const GeometryInfo& info = {});
        ref_ptr<Node> createCapsule(const GeometryInfo& info = {});
        ref_ptr<Node> createCone(const GeometryInfo& info = {});
        ref_ptr<Node> createCylinder(const GeometryInfo& info = {});
        ref_ptr<Node> createQuad(const GeometryInfo& info = {});
        ref_ptr<Node> createSphere(const GeometryInfo& info = {});

    private:
        uint32_t _allocatedTextureCount = 0;
        uint32_t _maxNumTextures = 0;
        ref_ptr<CompileTraversal> _compile;

        ref_ptr<DescriptorSetLayout> _descriptorSetLayout;
        ref_ptr<PipelineLayout> _pipelineLayout;
        ref_ptr<BindGraphicsPipeline> _bindGraphicsPipeline;

        std::map<vec4, ref_ptr<vec4Array2D>> _colorData;
        std::map<ref_ptr<Data>, ref_ptr<BindDescriptorSets>> _textureDescriptorSets;

        vec3 y_texcoord(const GeometryInfo& info) const;

        ref_ptr<BindGraphicsPipeline> _createGraphicsPipeline();
        ref_ptr<BindDescriptorSets> _createTexture(const GeometryInfo& info);

        using GeometryMap = std::map<GeometryInfo, ref_ptr<Node>>;
        GeometryMap _boxes;
        GeometryMap _capsules;
        GeometryMap _cones;
        GeometryMap _cylinders;
        GeometryMap _quads;
        GeometryMap _spheres;
    };

} // namespace vsg
