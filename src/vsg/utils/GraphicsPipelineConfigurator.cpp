/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>

using namespace vsg;

struct AssignGraphicsPipelineStates : public vsg::Visitor
{
    vsg::GraphicsPipelineConfigurator* config = nullptr;

    explicit AssignGraphicsPipelineStates(vsg::GraphicsPipelineConfigurator* in_config) :
        config(in_config) {}

    void apply(vsg::ColorBlendState& cbs) override { config->colorBlendState = ColorBlendState::create(cbs); }
    void apply(vsg::DepthStencilState& mss) override { config->depthStencilState = DepthStencilState::create(mss); }
    void apply(vsg::DynamicState& mss) override { config->dynamicState = DynamicState::create(mss); }
    void apply(vsg::InputAssemblyState& ias) override { config->inputAssemblyState = InputAssemblyState::create(ias); }
    void apply(vsg::MultisampleState& mss) override { config->multisampleState = MultisampleState::create(mss); }
    void apply(vsg::RasterizationState& rs) override { config->rasterizationState = RasterizationState::create(rs); }
    void apply(vsg::TessellationState& rs) override { config->tessellationState = TessellationState::create(rs); }
    void apply(vsg::VertexInputState& ias) override { config->vertexInputState = VertexInputState::create(ias); }
    void apply(vsg::ViewportState& ias) override { config->viewportState = ViewportState::create(ias); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GraphicsPipelineConfigurator
//
GraphicsPipelineConfigurator::GraphicsPipelineConfigurator(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
    // apply defaults
    auto& graphicsPipelineStates = shaderSet->defaultGraphicsPipelineStates;
    if (!graphicsPipelineStates.empty())
    {
        AssignGraphicsPipelineStates agps(this);
        for (auto& gps : graphicsPipelineStates)
        {
            gps->accept(agps);
        }
    }

    if (!vertexInputState) vertexInputState = VertexInputState::create();
    if (!inputAssemblyState) inputAssemblyState = vsg::InputAssemblyState::create();
    if (!rasterizationState) rasterizationState = vsg::RasterizationState::create();
    if (!colorBlendState) colorBlendState = vsg::ColorBlendState::create();
    if (!multisampleState) multisampleState = vsg::MultisampleState::create();
    if (!depthStencilState) depthStencilState = vsg::DepthStencilState::create();

    shaderHints = vsg::ShaderCompileSettings::create();
}

void GraphicsPipelineConfigurator::reset()
{
    vertexInputState->vertexAttributeDescriptions.clear();
    vertexInputState->vertexBindingDescriptions.clear();
    descriptorBindings.clear();
    descriptorSetLayout = {};
    shaderHints->defines.clear();
}

bool GraphicsPipelineConfigurator::enableArray(const std::string& name, VkVertexInputRate vertexInputRate, uint32_t stride, VkFormat format)
{
    auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        // set up bindings
        uint32_t bindingIndex = baseAttributeBinding + static_cast<uint32_t>(vertexInputState->vertexAttributeDescriptions.size());
        if (!attributeBinding.define.empty()) shaderHints->defines.insert(attributeBinding.define);
        vertexInputState->vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{attributeBinding.location, bindingIndex, (format != VK_FORMAT_UNDEFINED) ? format : attributeBinding.format, 0});
        vertexInputState->vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{bindingIndex, stride, vertexInputRate});
        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::enableTexture(const std::string& name)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!textureBinding.define.empty()) shaderHints->defines.insert(textureBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags, nullptr});

        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::enableUniform(const std::string& name)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!uniformBinding.define.empty()) shaderHints->defines.insert(uniformBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags, nullptr});

        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::assignArray(DataList& arrays, const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array)
{
    auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        VkFormat format = array ? array->properties.format : VK_FORMAT_UNDEFINED;

        // set up bindings
        uint32_t bindingIndex = baseAttributeBinding + static_cast<uint32_t>(arrays.size());
        if (!attributeBinding.define.empty()) shaderHints->defines.insert(attributeBinding.define);
        vertexInputState->vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{attributeBinding.location, bindingIndex, (format != VK_FORMAT_UNDEFINED) ? format : attributeBinding.format, 0});
        vertexInputState->vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{bindingIndex, array->properties.stride, vertexInputRate});

        arrays.push_back(array);
        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::assignTexture(Descriptors& descriptors, const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!textureBinding.define.empty()) shaderHints->defines.insert(textureBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags, nullptr});

        if (!sampler) sampler = Sampler::create();

        // create texture image and associated DescriptorSets and binding
        auto texture = vsg::DescriptorImage::create(sampler, textureData ? textureData : textureBinding.data, textureBinding.binding, 0, textureBinding.descriptorType);
        descriptors.push_back(texture);
        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::assignUniform(Descriptors& descriptors, const std::string& name, ref_ptr<Data> data)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!uniformBinding.define.empty()) shaderHints->defines.insert(uniformBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags, nullptr});

        auto uniform = vsg::DescriptorBuffer::create(data ? data : uniformBinding.data, uniformBinding.binding);
        descriptors.push_back(uniform);

        return true;
    }
    return false;
}

int GraphicsPipelineConfigurator::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(colorBlendState, rhs.colorBlendState))) return result;
    if ((result = compare_pointer(depthStencilState, rhs.depthStencilState))) return result;
    if ((result = compare_pointer(dynamicState, rhs.dynamicState))) return result;
    if ((result = compare_pointer(inputAssemblyState, rhs.inputAssemblyState))) return result;
    if ((result = compare_pointer(multisampleState, rhs.multisampleState))) return result;
    if ((result = compare_pointer(rasterizationState, rhs.rasterizationState))) return result;
    if ((result = compare_pointer(tessellationState, rhs.tessellationState))) return result;
    if ((result = compare_pointer(vertexInputState, rhs.vertexInputState))) return result;
    if ((result = compare_pointer(viewportState, rhs.viewportState))) return result;

    if ((result = compare_value(subpass, rhs.subpass))) return result;
    if ((result = compare_value(baseAttributeBinding, rhs.baseAttributeBinding))) return result;
    if ((result = compare_pointer(shaderSet, rhs.shaderSet))) return result;

    if ((result = compare_pointer(shaderHints, rhs.shaderHints))) return result;
    return compare_value_container(descriptorBindings, rhs.descriptorBindings);
}

void GraphicsPipelineConfigurator::init()
{
    if (!descriptorSetLayout)
    {
        descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    }

    vsg::PushConstantRanges pushConstantRanges;
    for (auto& pcb : shaderSet->pushConstantRanges)
    {
        if (pcb.define.empty()) pushConstantRanges.push_back(pcb.range);
    }

    vsg::DescriptorSetLayouts desriptorSetLayouts{descriptorSetLayout};
    if (additionalDescriptorSetLayout) desriptorSetLayouts.push_back(additionalDescriptorSetLayout);

    layout = vsg::PipelineLayout::create(desriptorSetLayouts, pushConstantRanges);

    GraphicsPipelineStates pipelineStates;
    if (colorBlendState) pipelineStates.push_back(colorBlendState);
    if (depthStencilState) pipelineStates.push_back(depthStencilState);
    if (dynamicState) pipelineStates.push_back(dynamicState);
    if (inputAssemblyState) pipelineStates.push_back(inputAssemblyState);
    if (multisampleState) pipelineStates.push_back(multisampleState);
    if (rasterizationState) pipelineStates.push_back(rasterizationState);
    if (tessellationState) pipelineStates.push_back(tessellationState);
    if (vertexInputState) pipelineStates.push_back(vertexInputState);
    if (viewportState) pipelineStates.push_back(viewportState);

    graphicsPipeline = GraphicsPipeline::create(layout, shaderSet->getShaderStages(shaderHints), pipelineStates, subpass);
    bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorConfigurator
//
DescriptorConfigurator::DescriptorConfigurator(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
}

bool DescriptorConfigurator::assignTexture(const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!textureBinding.define.empty()) defines.insert(textureBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags, nullptr});

        if (!sampler) sampler = Sampler::create();

        // create texture image and associated DescriptorSets and binding
        auto texture = DescriptorImage::create(sampler, textureData ? textureData : textureBinding.data, textureBinding.binding, 0, textureBinding.descriptorType);
        descriptors.push_back(texture);
        return true;
    }
    return false;
}

bool DescriptorConfigurator::assignUniform(const std::string& name, ref_ptr<Data> data)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        // set up bindings
        if (!uniformBinding.define.empty()) defines.insert(uniformBinding.define);
        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags, nullptr});

        auto uniform = DescriptorBuffer::create(data ? data : uniformBinding.data, uniformBinding.binding);
        descriptors.push_back(uniform);

        return true;
    }
    return false;
}

void DescriptorConfigurator::init()
{
    if (!descriptorBindings.empty())
    {
        auto descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);
        descriptorSet = DescriptorSet::create(descriptorSetLayout, descriptors);
    }
}
