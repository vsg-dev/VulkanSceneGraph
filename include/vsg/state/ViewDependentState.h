#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/io/Logger.h>
#include <vsg/lighting/Light.h>
#include <vsg/nodes/Switch.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>

namespace vsg
{

    /// ViewDescriptorSetLayout is a proxy class that uses the ViewDependentState's descriptorSetLayout as the DescriptorSetLayout to use.
    /// Used in pipelines that wish to utilize lights and other view dependent data provided by the View::viewDependentState.
    /// Use in combination with the BindViewDescriptorSet.
    class VSG_DECLSPEC ViewDescriptorSetLayout : public Inherit<DescriptorSetLayout, ViewDescriptorSetLayout>
    {
    public:
        ViewDescriptorSetLayout();

        VkDescriptorSetLayout vk(uint32_t deviceID) const override { return _viewDescriptorSetLayout ? _viewDescriptorSetLayout->vk(deviceID) : 0; }

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

    protected:
        ref_ptr<DescriptorSetLayout> _viewDescriptorSetLayout;
    };
    VSG_type_name(vsg::ViewDescriptorSetLayout);

    /// BindViewDescriptorSets is a proxy class that binds the View::viewDependentState's descriptorSet
    /// Used for passing lights and other view dependent state to the GPU.
    /// Use in conjunction with a pipeline configured with vsg::ViewDescriptorSetLayout.
    class VSG_DECLSPEC BindViewDescriptorSets : public Inherit<StateCommand, BindViewDescriptorSets>
    {
    public:
        BindViewDescriptorSets();

        BindViewDescriptorSets(VkPipelineBindPoint in_bindPoint, PipelineLayout* in_pipelineLayout, uint32_t in_firstSet) :
            Inherit(1 + in_firstSet),
            pipelineBindPoint(in_bindPoint),
            layout(in_pipelineLayout),
            firstSet(in_firstSet)
        {
        }

        // vkCmdBindDescriptorSets settings
        VkPipelineBindPoint pipelineBindPoint;
        ref_ptr<PipelineLayout> layout;
        uint32_t firstSet;

        int compare(const Object& rhs_object) const override;

        template<class N, class V>
        static void t_traverse(N& bds, V& visitor)
        {
            if (bds.layout) bds.layout->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindViewDescriptorSets() {}
    };
    VSG_type_name(vsg::BindViewDescriptorSets);

    // forward declare
    class ResourceRequirements;

    /// ViewDependentState to manage lighting, clip planes and texture projection
    /// By default assigned to the vsg::View, for standard usage you don't need to create or modify the ViewDependentState
    /// If you wish to override the standard lighting support provided by ViewDependentState you can subclass it.
    ///
    /// To leverage the state that the ViewDependentState provides you need to set up the graphics pipelines with the vsg::ViewDescriptorSetLayout,
    /// and add a vsg::BindViewDescriptorSet to a StateGroup.  You don't need to explicitly add these if you have created your scene graph using
    /// vsg::Builder or used loaders like vsgXchange::Assimp.
    class VSG_DECLSPEC ViewDependentState : public Inherit<Object, ViewDependentState>
    {
    public:
        explicit ViewDependentState(View* in_view);

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            node.descriptorSet->accept(visitor);
            if (node.preRenderCommandGraph) node.preRenderCommandGraph->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& rt) const override;

        // containers filled in by RecordTraversal
        std::vector<std::pair<dmat4, const AmbientLight*>> ambientLights;
        std::vector<std::pair<dmat4, const DirectionalLight*>> directionalLights;
        std::vector<std::pair<dmat4, const PointLight*>> pointLights;
        std::vector<std::pair<dmat4, const SpotLight*>> spotLights;

        virtual void init(ResourceRequirements& requirements);
        virtual void update(ResourceRequirements& requirements);

        virtual void clear();
        virtual void bindDescriptorSets(CommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet);

        virtual void compile(Context& context);

        View* view = nullptr;

        ref_ptr<vec4Array> lightData;
        ref_ptr<BufferInfo> lightDataBufferInfo;
        ref_ptr<DescriptorBuffer> lightDataDescriptor;

        ref_ptr<vec4Array> viewportData;
        ref_ptr<BufferInfo> viewportDataBufferInfo;
        ref_ptr<DescriptorBuffer> viewportDescriptor;

        ref_ptr<Image> shadowDepthImage;
        ref_ptr<DescriptorImage> shadowMapImages;
        ref_ptr<DescriptorImage> shadowMapDirectSamplerDescriptor;
        ref_ptr<DescriptorImage> shadowMapShadowSamplerDescriptor;

        ref_ptr<DescriptorSetLayout> descriptorSetLayout;
        ref_ptr<DescriptorSet> descriptorSet;

        // shadow map hints
        double maxShadowDistance = 1e8;
        double shadowMapBias = 0.005;
        double lambda = 0.5;

        // Shadow backend.
        bool compiled = false;
        ref_ptr<CommandGraph> preRenderCommandGraph;
        ref_ptr<Switch> preRenderSwitch;

        struct ShadowMap
        {
            ref_ptr<RenderGraph> renderGraph;
            ref_ptr<View> view;
        };

        mutable std::vector<ShadowMap> shadowMaps;

    protected:
        ~ViewDependentState();
    };
    VSG_type_name(vsg::ViewDependentState);

} // namespace vsg
