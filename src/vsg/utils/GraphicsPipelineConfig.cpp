/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/utils/GraphicsPipelineConfig.h>

using namespace vsg;

GraphicsPipelineConfig::GraphicsPipelineConfig(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
    vertexInputState = VertexInputState::create();
    inputAssemblyState = vsg::InputAssemblyState::create();
    rasterizationState = vsg::RasterizationState::create();
    colorBlendState = vsg::ColorBlendState::create();
    multisampleState = vsg::MultisampleState::create();
    depthStencilState = vsg::DepthStencilState::create();

    shaderHints = vsg::ShaderCompileSettings::create();
}

bool GraphicsPipelineConfig::assignArray(DataList& arrays, const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array)
{
    auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        if (!attributeBinding.define.empty()) shaderHints->defines.push_back(attributeBinding.define);

        uint32_t bindingIndex = baseAttributeBinding + static_cast<uint32_t>(arrays.size());
        vertexInputState->vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{attributeBinding.location, bindingIndex, attributeBinding.format, 0});
        vertexInputState->vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{bindingIndex, array->getLayout().stride, vertexInputRate});
        arrays.push_back(array);
        return true;
    }
    return false;
}

bool GraphicsPipelineConfig::assignTexture(Descriptors& descriptors, const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        if (!sampler) sampler = Sampler::create();

        if (!textureBinding.define.empty()) shaderHints->defines.push_back(textureBinding.define);

        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags, nullptr});

        // create texture image and associated DescriptorSets and binding
        auto texture = vsg::DescriptorImage::create(sampler, textureData ? textureData : textureBinding.data, textureBinding.binding, 0, textureBinding.descriptorType);
        descriptors.push_back(texture);
        return true;
    }
    return false;
}

bool GraphicsPipelineConfig::assignUniform(Descriptors& descriptors, const std::string& name, ref_ptr<Data> data)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        if (!uniformBinding.define.empty()) shaderHints->defines.push_back(uniformBinding.define);

        descriptorBindings.push_back(VkDescriptorSetLayoutBinding{uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags, nullptr});

        auto uniform = vsg::DescriptorBuffer::create(data ? data : uniformBinding.data, uniformBinding.binding);
        descriptors.push_back(uniform);

        return true;
    }
    return false;
}

void GraphicsPipelineConfig::init()
{
    descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    vsg::PushConstantRanges pushConstantRanges;
    for(auto& pcb : shaderSet->pushConstantRanges)
    {
        if (pcb.define.empty()) pushConstantRanges.push_back(pcb.range);
    }

    layout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);

    GraphicsPipelineStates pipelineStates{vertexInputState, inputAssemblyState, rasterizationState, colorBlendState, multisampleState, depthStencilState};

    graphicsPipeline = GraphicsPipeline::create(layout, shaderSet->getShaderStages(shaderHints), pipelineStates, subpass);
    bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);
}


int GraphicsPipelineConfig::compare(const Object& rhs) const
{
    return Object::compare(rhs);
}
