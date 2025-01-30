/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/io/Input.h>
#include <vsg/io/Output.h>
#include <vsg/io/read.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/TessellationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/state/material.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/Context.h>

#include "shaders/flat_ShaderSet.cpp"
#include "shaders/pbr_ShaderSet.cpp"
#include "shaders/phong_ShaderSet.cpp"

using namespace vsg;

int AttributeBinding::compare(const AttributeBinding& rhs) const
{
    if (name < rhs.name) return -1;
    if (name > rhs.name) return 1;

    if (define < rhs.define) return -1;
    if (define > rhs.define) return 1;

    int result = compare_value(location, rhs.location);
    if (result) return result;

    if ((result = compare_value(format, rhs.format))) return result;
    return compare_pointer(data, rhs.data);
}

int DescriptorBinding::compare(const DescriptorBinding& rhs) const
{
    if (name < rhs.name) return -1;
    if (name > rhs.name) return 1;

    if (define < rhs.define) return -1;
    if (define > rhs.define) return 1;

    int result = compare_value(set, rhs.set);
    if (result) return result;

    if ((result = compare_value(binding, rhs.binding))) return result;
    if ((result = compare_value(descriptorType, rhs.descriptorType))) return result;
    if ((result = compare_value(descriptorCount, rhs.descriptorCount))) return result;
    if ((result = compare_value(stageFlags, rhs.stageFlags))) return result;
    return compare_pointer(data, rhs.data);
}

int PushConstantRange::compare(const PushConstantRange& rhs) const
{
    if (name < rhs.name) return -1;
    if (name > rhs.name) return 1;

    if (define < rhs.define) return -1;
    if (define > rhs.define) return 1;

    return compare_region(range, range, rhs.range);
}

int DefinesArrayState::compare(const DefinesArrayState& rhs) const
{
    int result = compare_container(defines, rhs.defines);
    if (result) return result;

    return compare_pointer(arrayState, rhs.arrayState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CustomDescriptorSetBinding
//
CustomDescriptorSetBinding::CustomDescriptorSetBinding(uint32_t in_set) :
    set(in_set)
{
}

int CustomDescriptorSetBinding::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(set, rhs.set);
}

void CustomDescriptorSetBinding::read(Input& input)
{
    Object::read(input);

    input.read("set", set);
}

void CustomDescriptorSetBinding::write(Output& output) const
{
    Object::write(output);

    output.write("set", set);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentStateBinding
//
ViewDependentStateBinding::ViewDependentStateBinding(uint32_t in_set) :
    Inherit(in_set),
    viewDescriptorSetLayout(ViewDescriptorSetLayout::create())
{
}

int ViewDependentStateBinding::compare(const Object& rhs) const
{
    return CustomDescriptorSetBinding::compare(rhs);
}

void ViewDependentStateBinding::read(Input& input)
{
    CustomDescriptorSetBinding::read(input);
}

void ViewDependentStateBinding::write(Output& output) const
{
    CustomDescriptorSetBinding::write(output);
}

bool ViewDependentStateBinding::compatibleDescriptorSetLayout(const DescriptorSetLayout& dsl) const
{
    return viewDescriptorSetLayout->compare(dsl) == 0;
}

ref_ptr<DescriptorSetLayout> ViewDependentStateBinding::createDescriptorSetLayout()
{
    return viewDescriptorSetLayout;
}

ref_ptr<StateCommand> ViewDependentStateBinding::createStateCommand(ref_ptr<PipelineLayout> layout)
{
    return BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ShaderSet
//
ShaderSet::ShaderSet()
{
}

ShaderSet::ShaderSet(const ShaderStages& in_stages, ref_ptr<ShaderCompileSettings> in_hints) :
    stages(in_stages),
    defaultShaderHints(in_hints)
{
}

ShaderSet::~ShaderSet()
{
}

void ShaderSet::addAttributeBinding(const std::string& name, const std::string& define, uint32_t location, VkFormat format, ref_ptr<Data> data, CoordinateSpace coordinateSpace)
{
    attributeBindings.push_back(AttributeBinding{name, define, location, format, coordinateSpace, data});
}

void ShaderSet::addDescriptorBinding(const std::string& name, const std::string& define, uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Data> data, CoordinateSpace coordinateSpace)
{
    descriptorBindings.push_back(DescriptorBinding{name, define, set, binding, descriptorType, descriptorCount, stageFlags, coordinateSpace, data});
}

void ShaderSet::addPushConstantRange(const std::string& name, const std::string& define, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
    pushConstantRanges.push_back(vsg::PushConstantRange{name, define, VkPushConstantRange{stageFlags, offset, size}});
}

const AttributeBinding& ShaderSet::getAttributeBinding(const std::string& name) const
{
    for (auto& binding : attributeBindings)
    {
        if (binding.name == name) return binding;
    }
    return _nullAttributeBinding;
}

DescriptorBinding& ShaderSet::getDescriptorBinding(const std::string& name)
{
    for (auto& binding : descriptorBindings)
    {
        if (binding.name == name) return binding;
    }
    return _nullDescriptorBinding;
}

AttributeBinding& ShaderSet::getAttributeBinding(const std::string& name)
{
    for (auto& binding : attributeBindings)
    {
        if (binding.name == name) return binding;
    }
    return _nullAttributeBinding;
}

const DescriptorBinding& ShaderSet::getDescriptorBinding(const std::string& name) const
{
    for (const auto& binding : descriptorBindings)
    {
        if (binding.name == name) return binding;
    }
    return _nullDescriptorBinding;
}

ref_ptr<ArrayState> ShaderSet::getSuitableArrayState(const std::set<std::string>& defines) const
{
    // not all defines are relevant to the provided ArrayState
    // so check each one against the entries in the definesArrayState
    // relevant to the final matching.
    std::set<std::string> relevant_defines;
    for (auto& define : defines)
    {
        for (const auto& definesArrayState : definesArrayStates)
        {
            if (definesArrayState.defines.count(define) != 0)
            {
                relevant_defines.insert(define);
                break;
            }
        }
    }

    // find the matching ArrayState
    for (const auto& definesArrayState : definesArrayStates)
    {
        if (definesArrayState.defines == relevant_defines)
        {
            return definesArrayState.arrayState;
        }
    }

    return {};
}

ShaderStages ShaderSet::getShaderStages(ref_ptr<ShaderCompileSettings> scs)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (auto itr = variants.find(scs); itr != variants.end())
    {
        return itr->second;
    }

    auto& new_stages = variants[scs];
    for (auto& stage : stages)
    {
        if (vsg::compare_pointer(stage->module->hints, scs) == 0)
        {
            new_stages.push_back(stage);
        }
        else
        {
            auto new_stage = vsg::ShaderStage::create();
            new_stage->flags = stage->flags;
            new_stage->stage = stage->stage;
            new_stage->module = ShaderModule::create(stage->module->source, scs);
            new_stage->entryPointName = stage->entryPointName;
            new_stage->specializationConstants = stage->specializationConstants;
            new_stages.push_back(new_stage);
        }
    }

    return new_stages;
}

int ShaderSet::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer_container(stages, rhs.stages))) return result;
    if ((result = compare_container(attributeBindings, rhs.attributeBindings))) return result;
    if ((result = compare_container(descriptorBindings, rhs.descriptorBindings))) return result;
    if ((result = compare_container(pushConstantRanges, rhs.pushConstantRanges))) return result;
    if ((result = compare_container(definesArrayStates, rhs.definesArrayStates))) return result;
    if ((result = compare_container(optionalDefines, rhs.optionalDefines))) return result;
    return compare_pointer_container(defaultGraphicsPipelineStates, rhs.defaultGraphicsPipelineStates);
}

void ShaderSet::read(Input& input)
{
    Object::read(input);

    input.readObjects("stages", stages);

    if (input.version_greater_equal(1, 0, 4))
    {
        input.readObject("defaultShaderHints", defaultShaderHints);
    }

    auto num_attributeBindings = input.readValue<uint32_t>("attributeBindings");
    attributeBindings.resize(num_attributeBindings);
    for (auto& binding : attributeBindings)
    {
        input.read("name", binding.name);
        input.read("define", binding.define);
        input.read("location", binding.location);
        input.readValue<uint32_t>("format", binding.format);
        if (input.version_greater_equal(1, 1, 10)) input.readValue<uint32_t>("coordinateSpace", binding.coordinateSpace);
        input.readObject("data", binding.data);
    }

    auto num_descriptorBindings = input.readValue<uint32_t>(input.version_greater_equal(1, 0, 10) ? "descriptorBindings" : "uniformBindings");
    descriptorBindings.resize(num_descriptorBindings);
    for (auto& binding : descriptorBindings)
    {
        input.read("name", binding.name);
        input.read("define", binding.define);
        input.read("set", binding.set);
        input.read("binding", binding.binding);
        input.readValue<uint32_t>("descriptorType", binding.descriptorType);
        input.read("descriptorCount", binding.descriptorCount);
        input.readValue<uint32_t>("stageFlags", binding.stageFlags);
        if (input.version_greater_equal(1, 1, 10)) input.readValue<uint32_t>("coordinateSpace", binding.coordinateSpace);
        input.readObject("data", binding.data);
    }

    auto num_pushConstantRanges = input.readValue<uint32_t>("pushConstantRanges");
    pushConstantRanges.resize(num_pushConstantRanges);
    for (auto& pcr : pushConstantRanges)
    {
        input.read("name", pcr.name);
        input.read("define", pcr.define);
        input.readValue<uint32_t>("stageFlags", pcr.range.stageFlags);
        input.read("offset", pcr.range.offset);
        input.read("size", pcr.range.size);
    }

    auto num_definesArrayStates = input.readValue<uint32_t>("definesArrayStates");
    definesArrayStates.resize(num_definesArrayStates);
    for (auto& das : definesArrayStates)
    {
        input.readValues("defines", das.defines);
        input.readObject("arrayState", das.arrayState);
    }

    input.readValues("optionalDefines", optionalDefines);
    input.readObjects("defaultGraphicsPipelineStates", defaultGraphicsPipelineStates);

    auto num_variants = input.readValue<uint32_t>("variants");
    variants.clear();
    for (uint32_t i = 0; i < num_variants; ++i)
    {
        auto hints = input.readObject<ShaderCompileSettings>("hints");
        input.readObjects("stages", variants[hints]);
    }

    if (input.version_greater_equal(1, 0, 8))
    {
        auto num_custom = input.readValue<uint32_t>("customDescriptorSetBindings");
        customDescriptorSetBindings.clear();
        for (uint32_t i = 0; i < num_custom; ++i)
        {
            if (auto custom = input.readObject<CustomDescriptorSetBinding>("customDescriptorSetBinding"))
            {
                customDescriptorSetBindings.push_back(custom);
            }
        }
    }
}

void ShaderSet::write(Output& output) const
{
    Object::write(output);

    output.writeObjects("stages", stages);

    if (output.version_greater_equal(1, 0, 4))
    {
        output.writeObject("defaultShaderHints", defaultShaderHints);
    }

    output.writeValue<uint32_t>("attributeBindings", attributeBindings.size());
    for (auto& binding : attributeBindings)
    {
        output.write("name", binding.name);
        output.write("define", binding.define);
        output.write("location", binding.location);
        output.writeValue<uint32_t>("format", binding.format);
        if (output.version_greater_equal(1, 1, 10)) output.writeValue<uint32_t>("coordinateSpace", binding.coordinateSpace);
        output.writeObject("data", binding.data);
    }

    output.writeValue<uint32_t>(output.version_greater_equal(1, 0, 10) ? "descriptorBindings" : "uniformBindings", descriptorBindings.size());
    for (auto& binding : descriptorBindings)
    {
        output.write("name", binding.name);
        output.write("define", binding.define);
        output.write("set", binding.set);
        output.write("binding", binding.binding);
        output.writeValue<uint32_t>("descriptorType", binding.descriptorType);
        output.write("descriptorCount", binding.descriptorCount);
        output.writeValue<uint32_t>("stageFlags", binding.stageFlags);
        if (output.version_greater_equal(1, 1, 10)) output.writeValue<uint32_t>("coordinateSpace", binding.coordinateSpace);
        output.writeObject("data", binding.data);
    }

    output.writeValue<uint32_t>("pushConstantRanges", pushConstantRanges.size());
    for (auto& pcr : pushConstantRanges)
    {
        output.write("name", pcr.name);
        output.write("define", pcr.define);
        output.writeValue<uint32_t>("stageFlags", pcr.range.stageFlags);
        output.write("offset", pcr.range.offset);
        output.write("size", pcr.range.size);
    }

    output.writeValue<uint32_t>("definesArrayStates", definesArrayStates.size());
    for (const auto& das : definesArrayStates)
    {
        output.writeValues("defines", das.defines);
        output.writeObject("arrayState", das.arrayState);
    }

    output.writeValues("optionalDefines", optionalDefines);
    output.writeObjects("defaultGraphicsPipelineStates", defaultGraphicsPipelineStates);

    output.writeValue<uint32_t>("variants", variants.size());
    for (auto& [hints, variant_stages] : variants)
    {
        output.writeObject("hints", hints);
        output.writeObjects("stages", variant_stages);
    }

    if (output.version_greater_equal(1, 0, 8))
    {
        output.writeValue<uint32_t>("customDescriptorSetBindings", customDescriptorSetBindings.size());
        for (const auto& custom : customDescriptorSetBindings)
        {
            output.writeObject("customDescriptorSetBinding", custom);
        }
    }
}

ref_ptr<ShaderSet> vsg::createFlatShadedShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("flat"); itr != options->shaderSets.end()) return itr->second;
    }
    return flat_ShaderSet();
}

ref_ptr<ShaderSet> vsg::createPhongShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("phong"); itr != options->shaderSets.end()) return itr->second;
    }

    return phong_ShaderSet();
}

ref_ptr<ShaderSet> vsg::createPhysicsBasedRenderingShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("pbr"); itr != options->shaderSets.end()) return itr->second;
    }

    return pbr_ShaderSet();
}

std::pair<uint32_t, uint32_t> ShaderSet::descriptorSetRange() const
{
    if (descriptorBindings.empty()) return {0, 0};

    uint32_t minimum = std::numeric_limits<uint32_t>::max();
    uint32_t maximum = std::numeric_limits<uint32_t>::min();

    for (const auto& binding : descriptorBindings)
    {
        if (binding.set < minimum) minimum = binding.set;
        if (binding.set > maximum) maximum = binding.set;
    }

    return {minimum, maximum + 1};
}

bool ShaderSet::compatibleDescriptorSetLayout(const DescriptorSetLayout& dsl, const std::set<std::string>& defines, uint32_t set) const
{
    for (auto& cdsb : customDescriptorSetBindings)
    {
        if (cdsb->set == set && cdsb->compatibleDescriptorSetLayout(dsl)) return true;
    }

    DescriptorSetLayoutBindings bindings;
    for (auto& binding : descriptorBindings)
    {
        if (binding.set == set)
        {
            if (binding.define.empty() || defines.count(binding.define) > 0)
            {
                bindings.push_back(VkDescriptorSetLayoutBinding{binding.binding, binding.descriptorType, binding.descriptorCount, binding.stageFlags, nullptr});
            }
        }
    }

    return compare_value_container(dsl.bindings, bindings) == 0;
}

ref_ptr<DescriptorSetLayout> ShaderSet::createDescriptorSetLayout(const std::set<std::string>& defines, uint32_t set) const
{
    for (auto& cdsb : customDescriptorSetBindings)
    {
        if (cdsb->set == set) return cdsb->createDescriptorSetLayout();
    }

    DescriptorSetLayoutBindings bindings;
    for (auto& binding : descriptorBindings)
    {
        if (binding.set == set)
        {
            if (binding.define.empty() || defines.count(binding.define) > 0)
            {
                bindings.push_back(VkDescriptorSetLayoutBinding{binding.binding, binding.descriptorType, binding.descriptorCount, binding.stageFlags, nullptr});
            }
        }
    }

    return DescriptorSetLayout::create(bindings);
}

bool ShaderSet::compatiblePipelineLayout(const PipelineLayout& layout, const std::set<std::string>& defines) const
{
    uint32_t set = 0;
    for (const auto& descriptorSetLayout : layout.setLayouts)
    {
        if (descriptorSetLayout && !compatibleDescriptorSetLayout(*descriptorSetLayout, defines, set))
        {
            return false;
        }
        ++set;
    }

    PushConstantRanges ranges;
    for (auto& pcr : pushConstantRanges)
    {
        if (pcr.define.empty() || defines.count(pcr.define) == 1)
        {
            ranges.push_back(pcr.range);
        }
    }

    if (compare_value_container(layout.pushConstantRanges, ranges) != 0)
    {
        return false;
    }

    return true;
}

ref_ptr<PipelineLayout> ShaderSet::createPipelineLayout(const std::set<std::string>& defines, std::pair<uint32_t, uint32_t> range) const
{
    DescriptorSetLayouts descriptorSetLayouts;

    uint32_t set = 0;
    for (; set < range.first; ++set)
    {
        descriptorSetLayouts.push_back(DescriptorSetLayout::create());
    }

    for (; set < range.second; ++set)
    {
        descriptorSetLayouts.push_back(createDescriptorSetLayout(defines, set));
    }

    PushConstantRanges activePushConstantRanges;
    for (auto& pcb : pushConstantRanges)
    {
        if (pcb.define.empty() || defines.count(pcb.define) != 0) activePushConstantRanges.push_back(pcb.range);
    }

    return vsg::PipelineLayout::create(descriptorSetLayouts, activePushConstantRanges);
}
