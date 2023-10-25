/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/visit.h>
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

    ref_ptr<ColorBlendState> colorBlendState;
    ref_ptr<DepthStencilState> depthStencilState;
    ref_ptr<DynamicState> dynamicState;
    ref_ptr<InputAssemblyState> inputAssemblyState;
    ref_ptr<MultisampleState> multisampleState;
    ref_ptr<RasterizationState> rasterizationState;
    ref_ptr<TessellationState> tessellationState;
    ref_ptr<VertexInputState> vertexInputState;
    ref_ptr<ViewportState> viewportState;

    void apply(vsg::Object& object) override { object.traverse(*this); }
    void apply(vsg::ColorBlendState& cbs) override
    {
        colorBlendState = ColorBlendState::create(cbs);
        config->pipelineStates.push_back(colorBlendState);
    }
    void apply(vsg::DepthStencilState& mss) override
    {
        depthStencilState = DepthStencilState::create(mss);
        config->pipelineStates.push_back(depthStencilState);
    }
    void apply(vsg::DynamicState& mss) override
    {
        dynamicState = DynamicState::create(mss);
        config->pipelineStates.push_back(dynamicState);
    }
    void apply(vsg::InputAssemblyState& ias) override
    {
        inputAssemblyState = InputAssemblyState::create(ias);
        config->pipelineStates.push_back(inputAssemblyState);
    }
    void apply(vsg::MultisampleState& mss) override
    {
        multisampleState = MultisampleState::create(mss);
        config->pipelineStates.push_back(multisampleState);
    }
    void apply(vsg::RasterizationState& rs) override
    {
        rasterizationState = RasterizationState::create(rs);
        config->pipelineStates.push_back(rasterizationState);
    }
    void apply(vsg::TessellationState& rs) override
    {
        tessellationState = TessellationState::create(rs);
        config->pipelineStates.push_back(tessellationState);
    }
    void apply(vsg::VertexInputState& ias) override
    {
        vertexInputState = VertexInputState::create(ias);
        config->pipelineStates.push_back(vertexInputState);
    }
    void apply(vsg::ViewportState& ias) override
    {
        viewportState = ViewportState::create(ias);
        config->pipelineStates.push_back(viewportState);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorConfigurator
//
DescriptorConfigurator::DescriptorConfigurator(ref_ptr<ShaderSet> in_shaderSet) :
    shaderSet(in_shaderSet)
{
}

void DescriptorConfigurator::reset()
{
    assigned.clear();
    defines.clear();
    descriptorSets.clear();
}

bool DescriptorConfigurator::enableTexture(const std::string& name)
{
    if (auto& textureBinding = shaderSet->getDescriptorBinding(name))
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
    if (auto& textureBinding = shaderSet->getDescriptorBinding(name))
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
    if (auto& textureBinding = shaderSet->getDescriptorBinding(name))
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

bool DescriptorConfigurator::enableDescriptor(const std::string& name)
{
    if (auto& descriptorBinding = shaderSet->getDescriptorBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!descriptorBinding.define.empty()) defines.insert(descriptorBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(descriptorBinding.set, descriptorBinding.binding, descriptorBinding.descriptorType, descriptorBinding.descriptorCount, descriptorBinding.stageFlags,
                                DescriptorBuffer::create(descriptorBinding.data, descriptorBinding.binding));
    }
    return false;
}

bool DescriptorConfigurator::assignDescriptor(const std::string& name, ref_ptr<Data> data, uint32_t dstArrayElement)
{
    if (auto& descriptorBinding = shaderSet->getDescriptorBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!descriptorBinding.define.empty()) defines.insert(descriptorBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(descriptorBinding.set, descriptorBinding.binding, descriptorBinding.descriptorType, descriptorBinding.descriptorCount, descriptorBinding.stageFlags,
                                DescriptorBuffer::create(data ? data : descriptorBinding.data, descriptorBinding.binding, dstArrayElement, descriptorBinding.descriptorType));
    }
    return false;
}

bool DescriptorConfigurator::assignDescriptor(const std::string& name, const BufferInfoList& bufferInfoList, uint32_t dstArrayElement)
{
    if (auto& descriptorBinding = shaderSet->getDescriptorBinding(name))
    {
        assigned.insert(name);

        // set up bindings
        if (!descriptorBinding.define.empty()) defines.insert(descriptorBinding.define);

        // create uniform and associated DescriptorSets and binding
        return assignDescriptor(descriptorBinding.set, descriptorBinding.binding, descriptorBinding.descriptorType, descriptorBinding.descriptorCount, descriptorBinding.stageFlags,
                                DescriptorBuffer::create(bufferInfoList, descriptorBinding.binding, dstArrayElement, descriptorBinding.descriptorType));
    }
    return false;
}

bool DescriptorConfigurator::assignDescriptor(uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Descriptor> descriptor)
{
    if (auto currentSize = descriptorSets.size(); set >= currentSize)
    {
        descriptorSets.resize(set + 1);
        for (auto i = currentSize; i <= set; i++)
        {
            descriptorSets[i] = vsg::DescriptorSet::create();
            descriptorSets[i]->setLayout = DescriptorSetLayout::create();
        }
    }

    auto& ds = descriptorSets[set];
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
        for (auto& descriptorBinding : shaderSet->descriptorBindings)
        {
            if (descriptorBinding.define.empty() && assigned.count(descriptorBinding.name) == 0)
            {
                bool set_matched = false;
                for (auto& cds : shaderSet->customDescriptorSetBindings)
                {
                    if (cds->set == descriptorBinding.set)
                    {
                        set_matched = true;
                        break;
                    }
                }
                if (!set_matched)
                {
                    bool isTexture = false;
                    switch (descriptorBinding.descriptorType)
                    {
                    case (VK_DESCRIPTOR_TYPE_SAMPLER):
                    case (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER):
                    case (VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE):
                    case (VK_DESCRIPTOR_TYPE_STORAGE_IMAGE):
                        isTexture = true;
                        break;
                    default:
                        break;
                    }

                    if (isTexture)
                    {
                        assignDescriptor(descriptorBinding.set, descriptorBinding.binding, descriptorBinding.descriptorType, descriptorBinding.descriptorCount, descriptorBinding.stageFlags,
                                         DescriptorImage::create(Sampler::create(), descriptorBinding.data, descriptorBinding.binding, 0, descriptorBinding.descriptorType));
                    }
                    else
                    {
                        assignDescriptor(descriptorBinding.set, descriptorBinding.binding, descriptorBinding.descriptorType, descriptorBinding.descriptorCount, descriptorBinding.stageFlags,
                                         DescriptorBuffer::create(descriptorBinding.data, descriptorBinding.binding));
                    }

                    assigned.insert(descriptorBinding.name);
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
    _assignShaderSetSettings();
}

void GraphicsPipelineConfigurator::_assignShaderSetSettings()
{
    // apply defaults
    AssignGraphicsPipelineStates agps(this);
    for (auto& pipelineState : shaderSet->defaultGraphicsPipelineStates)
    {
        pipelineState->accept(agps);
    }

    if (!agps.vertexInputState) pipelineStates.push_back(VertexInputState::create());
    if (!agps.inputAssemblyState) pipelineStates.push_back(vsg::InputAssemblyState::create());
    if (!agps.rasterizationState) pipelineStates.push_back(vsg::RasterizationState::create());
    if (!agps.colorBlendState) pipelineStates.push_back(vsg::ColorBlendState::create());
    if (!agps.multisampleState) pipelineStates.push_back(vsg::MultisampleState::create());
    if (!agps.depthStencilState) pipelineStates.push_back(vsg::DepthStencilState::create());

    shaderHints = shaderSet->defaultShaderHints ? vsg::ShaderCompileSettings::create(*shaderSet->defaultShaderHints) : vsg::ShaderCompileSettings::create();
}

void GraphicsPipelineConfigurator::traverse(Visitor& visitor)
{
    for (auto& ps : pipelineStates) ps->accept(visitor);
    if (shaderSet) shaderSet->accept(visitor);
    if (shaderHints) shaderHints->accept(visitor);
    if (descriptorConfigurator) descriptorConfigurator->accept(visitor);
}

void GraphicsPipelineConfigurator::traverse(ConstVisitor& visitor) const
{
    for (auto& ps : pipelineStates) ps->accept(visitor);
    if (shaderSet) shaderSet->accept(visitor);
    if (shaderHints) shaderHints->accept(visitor);
    if (descriptorConfigurator) descriptorConfigurator->accept(visitor);
}

void GraphicsPipelineConfigurator::reset()
{
    pipelineStates.clear();
    shaderHints->defines.clear();
    if (descriptorConfigurator) descriptorConfigurator->reset();

    _assignShaderSetSettings();
}

struct SetPipelineStates : public Visitor
{
    uint32_t base = 0;
    const AttributeBinding& binding;
    VkVertexInputRate vir;
    uint32_t stride;
    VkFormat format;

    SetPipelineStates(uint32_t in_base, const AttributeBinding& in_binding, VkVertexInputRate in_vir, uint32_t in_stride, VkFormat in_format) :
        base(in_base),
        binding(in_binding),
        vir(in_vir),
        stride(in_stride),
        format(in_format) {}

    void apply(Object& object) override { object.traverse(*this); }
    void apply(VertexInputState& vis) override
    {
        uint32_t bindingIndex = base + static_cast<uint32_t>(vis.vertexAttributeDescriptions.size());
        vis.vertexAttributeDescriptions.push_back(VkVertexInputAttributeDescription{binding.location, bindingIndex, (format != VK_FORMAT_UNDEFINED) ? format : binding.format, 0});
        vis.vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{bindingIndex, stride, vir});
    }
};

bool GraphicsPipelineConfigurator::enableArray(const std::string& name, VkVertexInputRate vertexInputRate, uint32_t stride, VkFormat format)
{
    auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        if (!attributeBinding.define.empty()) shaderHints->defines.insert(attributeBinding.define);

        SetPipelineStates setVertexAttributeState(baseAttributeBinding, attributeBinding, vertexInputRate, stride, format);
        accept(setVertexAttributeState);
        return true;
    }
    return false;
}

bool GraphicsPipelineConfigurator::enableTexture(const std::string& name)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->enableTexture(name);
}

bool GraphicsPipelineConfigurator::enableDescriptor(const std::string& name)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->enableDescriptor(name);
}

bool GraphicsPipelineConfigurator::assignArray(DataList& arrays, const std::string& name, VkVertexInputRate vertexInputRate, ref_ptr<Data> array)
{
    const auto& attributeBinding = shaderSet->getAttributeBinding(name);
    if (attributeBinding)
    {
        VkFormat format = array ? array->properties.format : VK_FORMAT_UNDEFINED;

        SetPipelineStates setVertexAttributeState(baseAttributeBinding, attributeBinding, vertexInputRate, array->properties.stride, format);
        accept(setVertexAttributeState);

        arrays.push_back(array);
        return true;
    }
    return false;
}
#if 0
bool GraphicsPipelineConfigurator::assignTexture(const std::string& name, ref_ptr<Data> textureData, ref_ptr<Sampler> sampler)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->assignTexture(name, textureData, sampler);
}

bool GraphicsPipelineConfigurator::assignDescriptor(const std::string& name, ref_ptr<Data> data)
{
    if (!descriptorConfigurator) descriptorConfigurator = DescriptorConfigurator::create(shaderSet);
    return descriptorConfigurator->assignDescriptor(name, data);
}
#endif
int GraphicsPipelineConfigurator::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer_container(pipelineStates, rhs.pipelineStates))) return result;

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
    graphicsPipeline = GraphicsPipeline::create(layout, shaderSet->getShaderStages(shaderHints), pipelineStates, subpass);
    bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);
}

// Determine if a descriptor set contains null values and therefore should not be written, updated,
// or bound. This might conflict someday with Vulkan features and extensions for null descriptors.

class DescriptorSetContainsNull : public ConstVisitor
{
public:
    bool containsNull = false;
    void apply(const DescriptorSet& ds) override
    {
        if (ds.descriptors.empty())
        {
            containsNull = true;
        }
        else
        {
            ds.traverse(*this);
        }
    }

    void apply(const DescriptorBuffer& db) override
    {
        containsNull = containsNull || db.bufferInfoList.empty();
    }

    void apply(const DescriptorImage& di) override
    {
        containsNull = containsNull || di.imageInfoList.empty();
    }
    // DescriptorTexelBufferView?
};

// The inverse, mainly for issuing a warning if a descriptor set contains nulls and values

class DescriptorSetContainsData : public ConstVisitor
{
public:
    bool containsData = false;
    void apply(const DescriptorSet& ds) override
    {
        ds.traverse(*this);
    }

    void apply(const DescriptorBuffer& db) override
    {
        containsData = containsData || !db.bufferInfoList.empty();
    }

    void apply(const DescriptorImage& di) override
    {
        containsData = containsData || !di.imageInfoList.empty();
    }
    // DescriptorTexelBufferView?
};

void GraphicsPipelineConfigurator::copyTo(ref_ptr<StateGroup> stateGroup, ref_ptr<SharedObjects> sharedObjects)
{
    // create StateGroup as the root of the scene/command graph to hold the GraphicsPipeline, and binding of Descriptors to decorate the whole graph
    if (sharedObjects) sharedObjects->share(bindGraphicsPipeline);

    stateGroup->add(bindGraphicsPipeline);

    if (descriptorConfigurator)
    {
        for (size_t set = 0; set < descriptorConfigurator->descriptorSets.size(); ++set)
        {
            if (auto ds = descriptorConfigurator->descriptorSets[set])
            {
                if (visit<DescriptorSetContainsNull>(ds).containsNull)
                {
                    if (visit<DescriptorSetContainsData>(ds).containsData)
                    {
                        warn("descriptor set contains null values and data");
                    }
                }
                else
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
