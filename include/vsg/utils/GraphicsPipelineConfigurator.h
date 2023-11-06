#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/DynamicState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/TessellationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>

namespace vsg
{

    /// DescriptorConfigurator utility provides a means of setting up descriptors using ShaderSet as a guide for required layouts/bindings.
    class VSG_DECLSPEC DescriptorConfigurator : public vsg::Inherit<Object, DescriptorConfigurator>
    {
    public:
        DescriptorConfigurator(ref_ptr<ShaderSet> in_shaderSet = {});

        ref_ptr<ShaderSet> shaderSet;
        bool blending = false;
        bool two_sided = false;

        int compare(const Object& rhs) const override;

        void reset();

        bool enableTexture(const std::string& name);
        bool assignTexture(const std::string& name, ref_ptr<Data> textureData = {}, ref_ptr<Sampler> sampler = {}, uint32_t dstArrayElement = 0);
        bool assignTexture(const std::string& name, const ImageInfoList& imageInfoList, uint32_t dstArrayElement = 0);

        bool enableDescriptor(const std::string& name);
        bool assignDescriptor(const std::string& name, ref_ptr<Data> data = {}, uint32_t dstArrayElement = 0);
        bool assignDescriptor(const std::string& name, const BufferInfoList& bufferInfoList, uint32_t dstArrayElement = 0);

        [[deprecated("use enableDescriptor(..)")]] bool enableUniform(const std::string& name) { return enableDescriptor(name); }

        [[deprecated("use assignDescriptor(..)")]] bool assignUniform(const std::string& name, ref_ptr<Data> data = {}, uint32_t dstArrayElement = 0) { return assignDescriptor(name, data, dstArrayElement); }

        [[deprecated("use assignDescriptor(..)")]] bool assignUniform(const std::string& name, const BufferInfoList& bufferInfoList, uint32_t dstArrayElement = 0) { return assignDescriptor(name, bufferInfoList, dstArrayElement); }

        bool assignDescriptor(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Descriptor> descriptor);

        /// call after all the textures/uniforms have been explictly assigned to add in textures/uniforms descriptors that are enabled by default (define == "").
        bool assignDefaults(const std::set<uint32_t>& inheritedSets = {});

        std::set<std::string> assigned;
        std::set<std::string> defines;
        std::vector<ref_ptr<DescriptorSet>> descriptorSets;
    };
    VSG_type_name(vsg::DescriptorConfigurator);

    /// ArrayConfigurator utility provides a means of setting up arrays using ShaderSet as a guide for required bindings.
    class VSG_DECLSPEC ArrayConfigurator : public vsg::Inherit<Object, ArrayConfigurator>
    {
    public:
        ArrayConfigurator(ref_ptr<ShaderSet> in_shaderSet = {});

        ref_ptr<ShaderSet> shaderSet;

        int compare(const Object& rhs) const override;
        bool assignArray(const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array);

        uint32_t baseAttributeBinding = 0;
        std::set<std::string> assigned;
        std::set<std::string> defines;
        /// VkPipelineVertexInputStateCreateInfo settings
        VertexInputState::Bindings vertexBindingDescriptions;
        VertexInputState::Attributes vertexAttributeDescriptions;

        DataList arrays;
    };
    VSG_type_name(vsg::ArrayConfigurator);

    /// GraphicsPipelineConfigurator utility provides a means of setting up state and geometry using ShaderSet as a guide for required layouts/bindings.
    class VSG_DECLSPEC GraphicsPipelineConfigurator : public vsg::Inherit<Object, GraphicsPipelineConfigurator>
    {
    public:
        GraphicsPipelineConfigurator(ref_ptr<ShaderSet> in_shaderSet = {});

        void traverse(Visitor& visitor) override;
        void traverse(ConstVisitor& visitor) const override;

        // inputs to setup of GraphicsPipeline, the default sets are taken from any provided by ShaderSet::defaultGraphicsPipelineStates
        GraphicsPipelineStates pipelineStates;

        uint32_t subpass = 0;
        uint32_t baseAttributeBinding = 0;
        ref_ptr<ShaderSet> shaderSet;

        void reset();

        bool enableArray(const std::string& name, VkVertexInputRate vertexInputRate, uint32_t stride, VkFormat format = VK_FORMAT_UNDEFINED);
        bool enableDescriptor(const std::string& name);
        bool enableTexture(const std::string& name);

        bool assignArray(DataList& arrays, const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array);
        bool assignDescriptor(const std::string& name, ref_ptr<Data> data = {}, uint32_t dstArrayElement = 0);
        bool assignDescriptor(const std::string& name, const BufferInfoList& bufferInfoList, uint32_t dstArrayElement = 0);
        bool assignTexture(const std::string& name, ref_ptr<Data> textureData = {}, ref_ptr<Sampler> sampler = {}, uint32_t dstArrayElement = 0);
        bool assignTexture(const std::string& name, const ImageInfoList& imageInfoList, uint32_t dstArrayElement = 0);

        /// updated the inhertedSets based in which of the inherted state are compatible.
        void inheritedState(const Object* object);

        [[deprecated("use enableDescriptor(..)")]] bool enableUniform(const std::string& name) { return enableDescriptor(name); }

        [[deprecated("use assignDescriptor(..)")]] bool assignUniform(const std::string& name, ref_ptr<Data> data = {}) { return assignDescriptor(name, data); }

        // setup by assign calls
        ref_ptr<ShaderCompileSettings> shaderHints;
        ref_ptr<DescriptorConfigurator> descriptorConfigurator;
        std::set<uint32_t> inheritedSets;

        int compare(const Object& rhs) const override;

        // initialize state objects
        virtual void init();

        // copy state objects to StateGroup
        virtual void copyTo(ref_ptr<StateGroup> stateGroup, ref_ptr<SharedObjects> sharedObjects = {});

        // setup by init()
        ref_ptr<PipelineLayout> layout;
        ref_ptr<GraphicsPipeline> graphicsPipeline;
        ref_ptr<BindGraphicsPipeline> bindGraphicsPipeline;

    protected:
        void _assignShaderSetSettings();
    };
    VSG_type_name(vsg::GraphicsPipelineConfigurator);

    /// provided for backwards compatibility
    using GraphicsPipelineConfig = GraphicsPipelineConfigurator;
    using DescriptorConfig = DescriptorConfigurator;

} // namespace vsg
