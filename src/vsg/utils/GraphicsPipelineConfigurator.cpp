/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/SharedObjects.h>

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
// DescriptorConfigurator
//
DescriptorConfigurator::DescriptorConfigurator(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
}

void DescriptorConfigurator::report()
{
    info("DescriptorConfigurator::report(", shaderSet, ") ", this);

    if (shaderSet)
    {
        for (auto& uniform : shaderSet->uniformBindings)
        {
            info("    uniform name = ", uniform.name, ", define = ", uniform.define, ", set = ", uniform.set, ", binding = ", uniform.binding, ", data = ", uniform.data);
        }

        for (auto& uniform : shaderSet->uniformBindings)
        {
            if (uniform.define.empty() && assigned.count(uniform.name) == 0)
            {
                bool set_matched = false;
                for (auto& cds : shaderSet->customDescriptorSetBindings)
                {
                    if (cds->set == uniform.set)
                    {
                        set_matched = true;
                        break;
                    }
                }
                if (!set_matched) info("   need to assign ", uniform.name, ", data = ", uniform.data);
            }
        }
    }

    for (auto& value : assigned)
    {
        info("    assigned ", value);
    }
    for (auto& value : defines)
    {
        info("    defines ", value);
    }
    for (auto& ds : descriptorSets)
    {
        info("    descriptorSet = ", ds);
    }
}

void DescriptorConfigurator::reset()
{
    assigned.clear();
    defines.clear();
    descriptorSets.clear();
}

bool DescriptorConfigurator::enableTexture(const std::string& name)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!textureBinding.define.empty()) defines.insert(textureBinding.define);
        auto sampler = Sampler::create();

        // create texture image and associated DescriptorSets and binding
        return assignDescriptor(textureBinding.set, textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags,
                                DescriptorImage::create(sampler, textureBinding.data, textureBinding.binding, 0, textureBinding.descriptorType));
    }
    return false;
}

bool DescriptorConfigurator::assignTexture(const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler, uint32_t dstArrayElement)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!textureBinding.define.empty()) defines.insert(textureBinding.define);
        if (!sampler) sampler = Sampler::create();

        // create texture image and associated DescriptorSets and binding
        return assignDescriptor(textureBinding.set, textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags,
                                DescriptorImage::create(sampler, textureData ? textureData : textureBinding.data, textureBinding.binding, dstArrayElement, textureBinding.descriptorType));
    }
    return false;
}

bool DescriptorConfigurator::assignTexture(const std::string& name, const ImageInfoList& imageInfoList, uint32_t dstArrayElement)
{
    if (auto& textureBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!textureBinding.define.empty()) defines.insert(textureBinding.define);

        // create texture image and associated DescriptorSets and binding
        return assignDescriptor(textureBinding.set, textureBinding.binding, textureBinding.descriptorType, textureBinding.descriptorCount, textureBinding.stageFlags,
                                DescriptorImage::create(imageInfoList, textureBinding.binding, dstArrayElement, textureBinding.descriptorType));
    }
    return false;
}

bool DescriptorConfigurator::enableUniform(const std::string& name)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!uniformBinding.define.empty()) defines.insert(uniformBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(uniformBinding.set, uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags,
                                DescriptorBuffer::create(uniformBinding.data, uniformBinding.binding));
    }
    return false;
}

bool DescriptorConfigurator::assignUniform(const std::string& name, ref_ptr<Data> data, uint32_t dstArrayElement)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!uniformBinding.define.empty()) defines.insert(uniformBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(uniformBinding.set, uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags,
                                DescriptorBuffer::create(data ? data : uniformBinding.data, uniformBinding.binding, dstArrayElement));
    }
    return false;
}

bool DescriptorConfigurator::assignUniform(const std::string& name, const BufferInfoList& bufferInfoList, uint32_t dstArrayElement)
{
    if (auto& uniformBinding = shaderSet->getUniformBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!uniformBinding.define.empty()) defines.insert(uniformBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(uniformBinding.set, uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags,
                                DescriptorBuffer::create(bufferInfoList, uniformBinding.binding, dstArrayElement));
    }
    return false;
}

bool DescriptorConfigurator::assignDescriptor(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Descriptor> descriptor)
{
    if (set >= descriptorSets.size()) descriptorSets.resize(set + 1);

    auto& ds = descriptorSets[set];
    if (!ds)
    {
        ds = vsg::DescriptorSet::create();
        ds->setLayout = DescriptorSetLayout::create();
    }

    ds->descriptors.push_back(descriptor);

    auto& descriptorBindings = ds->setLayout->bindings;
    descriptorBindings.push_back(VkDescriptorSetLayoutBinding{binding, descriptorType, descriptorCount, stageFlags, nullptr});

    return true;
}

bool DescriptorConfigurator::assignDefaults()
{
    bool assignedDefault = false;
    if (shaderSet)
    {
        for (auto& uniformBinding : shaderSet->uniformBindings)
        {
            if (uniformBinding.define.empty() && assigned.count(uniformBinding.name) == 0)
            {
                bool set_matched = false;
                for (auto& cds : shaderSet->customDescriptorSetBindings)
                {
                    if (cds->set == uniformBinding.set)
                    {
                        set_matched = true;
                        break;
                    }
                }
                if (!set_matched)
                {
                    bool isTexture = false;
                    switch(uniformBinding.descriptorType)
                    {
                        case(VK_DESCRIPTOR_TYPE_SAMPLER):
                        case(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER):
                        case(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE):
                        case(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE):
                            isTexture = true;
                            break;
                        default:
                            break;
                    }

                    if (isTexture)
                    {
                        assignDescriptor(uniformBinding.set, uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags,
                                         DescriptorImage::create(Sampler::create(), uniformBinding.data, uniformBinding.binding, 0, uniformBinding.descriptorType));
                    }
                    else
                    {
                        assignDescriptor(uniformBinding.set, uniformBinding.binding, uniformBinding.descriptorType, uniformBinding.descriptorCount, uniformBinding.stageFlags,
                                         DescriptorBuffer::create(uniformBinding.data, uniformBinding.binding));
                    }

                    assigned.insert(uniformBinding.name);
                    assignedDefault = true;
                }
            }
        }
    }

    return assignedDefault;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ArrayConfigurator
//
ArrayConfigurator::ArrayConfigurator(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
}

void ArrayConfigurator::report()
{
    info("DescriptorConfigurator::report(", shaderSet, ") ", this);

    if (shaderSet)
    {
        for (auto& attrib : shaderSet->attributeBindings)
        {
            info("    attrib name = ", attrib.name, ", define = ", attrib.define, ", location = ", attrib.location);
        }
    }
}

bool ArrayConfigurator::assignArray(const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array)
{
    auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        assigned.insert(name);

        VkFormat format = array ? array->properties.format : VK_FORMAT_UNDEFINED;

        // set up bindings
        uint32_t bindingIndex = baseAttributeBinding + static_cast<uint32_t>(arrays.size());
        if (!attributeBinding.define.empty()) defines.insert(attributeBinding.define);

        vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{attributeBinding.location, bindingIndex, (format != VK_FORMAT_UNDEFINED) ? format : attributeBinding.format, 0});
        vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{bindingIndex, array->properties.stride, vertexInputRate});

        arrays.push_back(array);
        return true;
    }
    return false;
}

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

    shaderHints = shaderSet->defaultShaderHints ? vsg::ShaderCompileSettings::create(*shaderSet->defaultShaderHints) : vsg::ShaderCompileSettings::create();
}

void GraphicsPipelineConfigurator::reset()
{
    vertexInputState->vertexAttributeDescriptions.clear();
    vertexInputState->vertexBindingDescriptions.clear();
    shaderHints->defines.clear();

    descriptorConfigurator = {};

    // if (descriptorConfigurator) descriptorConfigurator->reset();
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
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->enableTexture(name);
}

bool GraphicsPipelineConfigurator::enableUniform(const std::string& name)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->enableUniform(name);
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

bool GraphicsPipelineConfigurator::assignTexture(const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->assignTexture(name, textureData, sampler);
}

bool GraphicsPipelineConfigurator::assignUniform(const std::string& name, ref_ptr<Data> data)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->assignUniform(name, data);
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
    return compare_pointer(descriptorConfigurator, rhs.descriptorConfigurator);
}

void GraphicsPipelineConfigurator::init()
{
    vsg::PushConstantRanges pushConstantRanges;
    for (auto& pcb : shaderSet->pushConstantRanges)
    {
        if (pcb.define.empty()) pushConstantRanges.push_back(pcb.range);
    }

    vsg::DescriptorSetLayouts desriptorSetLayouts;

    if (descriptorConfigurator)
    {
        descriptorConfigurator->assignDefaults();

        shaderHints->defines.insert(descriptorConfigurator->defines.begin(), descriptorConfigurator->defines.end());
        for (auto& ds : descriptorConfigurator->descriptorSets)
        {
            desriptorSetLayouts.push_back(ds->setLayout);
        }
    }

    for (auto& cds : shaderSet->customDescriptorSetBindings)
    {
        desriptorSetLayouts.push_back(cds->createDescriptorSetLayout());
    }

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

void GraphicsPipelineConfigurator::copyTo(ref_ptr<StateGroup> stateGroup, ref_ptr<SharedObjects> sharedObjects)
{
    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    if (sharedObjects) sharedObjects->share(bindGraphicsPipeline);

    stateGroup->add(bindGraphicsPipeline);

    if (descriptorConfigurator)
    {
        for (size_t set = 0; set < descriptorConfigurator->descriptorSets.size(); ++set)
        {
            if (auto ds = descriptorConfigurator->descriptorSets[set])
            {
                if (sharedObjects)
                {
                    sharedObjects->share(ds);
                }

                auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, static_cast<uint32_t>(set), ds);
                if (sharedObjects)
                {
                    sharedObjects->share(bindDescriptorSet);
                }

                stateGroup->add(bindDescriptorSet);
            }
        }
    }

    for (auto& cds : shaderSet->customDescriptorSetBindings)
    {
        if (auto sc = cds->createStateCommand(layout))
        {
            if (sharedObjects)
            {
                sharedObjects->share(sc);
            }
            stateGroup->add(sc);
        }
    }

    // assign any custom ArrayState that may be required.
    stateGroup->prototypeArrayState = shaderSet->getSuitableArrayState(shaderHints->defines);
}
