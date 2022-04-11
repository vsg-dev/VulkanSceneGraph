/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/io/read.h>
#include <vsg/state/material.h>
#include <vsg/utils/ShaderSet.h>

#include "shaders/assimp_flat_shaded_frag.cpp"
#include "shaders/assimp_pbr_frag.cpp"
#include "shaders/assimp_phong_frag.cpp"
#include "shaders/assimp_vert.cpp"

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

int UniformBinding::compare(const UniformBinding& rhs) const
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

ShaderSet::ShaderSet()
{
}

ShaderSet::ShaderSet(const ShaderStages& in_stages) :
    stages(in_stages)
{
}

ShaderSet::~ShaderSet()
{
}

void ShaderSet::addAttributeBinding(std::string name, std::string define, uint32_t location, VkFormat format, ref_ptr<Data> data)
{
    attributeBindings.push_back(AttributeBinding{name, define, location, format, data});
}

void ShaderSet::addUniformBinding(std::string name, std::string define, uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Data> data)
{
    uniformBindings.push_back(UniformBinding{name, define, set, binding, descriptorType, descriptorCount, stageFlags, data});
}

void ShaderSet::addPushConstantRange(std::string name, std::string define, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
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

const UniformBinding& ShaderSet::getUniformBinding(const std::string& name) const
{
    for (auto& binding : uniformBindings)
    {
        if (binding.name == name) return binding;
    }
    return _nullUniformBinding;
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

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer_container(stages, rhs.stages)) )return result;
    if ((result = compare_container(attributeBindings, rhs.attributeBindings))) return result;
    if ((result = compare_container(uniformBindings, rhs.uniformBindings))) return result;
    return compare_container(pushConstantRanges, rhs.pushConstantRanges);
}

void ShaderSet::read(Input& input)
{
    Object::read(input);

    input.readObjects("stages", stages);

    auto num_attributeBindings = input.readValue<uint32_t>("attributeBindings");
    auto num_uniformBindings = input.readValue<uint32_t>("uniformBindings");
    auto num_pushConstantRanges = input.readValue<uint32_t>("pushConstantRanges");
    auto num_variants = input.readValue<uint32_t>("variants");

    variants.clear();
    for (uint32_t i = 0; i < num_variants; ++i)
    {
        auto hints = input.readObject<ShaderCompileSettings>("hints");
        input.readObjects("stages", variants[hints]);
    }

#if 0
    std::vector<AttributeBinding> attributeBindings;
    std::vector<UniformBinding> uniformBindings;
    std::vector<PushConstantRange> pushConstantRanges;

    /// variants of the rootShaderModule compiled for differen combinations of ShaderCompileSettings
    std::map<ref_ptr<ShaderCompileSettings>, ShaderStages, DerefenceLess> variants;
#endif
}

void ShaderSet::write(Output& output) const
{
    Object::write(output);

    output.writeObjects("stages", stages);

    output.writeValue<uint32_t>("attributeBindings", attributeBindings.size());
    output.writeValue<uint32_t>("uniformBindings", uniformBindings.size());
    output.writeValue<uint32_t>("pushConstantRanges", pushConstantRanges.size());
    output.writeValue<uint32_t>("variants", variants.size());
    for (auto& [hints, variant_stages] : variants)
    {
        output.writeObject("hints", hints);
        output.writeObjects("stages", variant_stages);
    }
}

VSG_DECLSPEC ref_ptr<ShaderSet> vsg::createFlatShadedShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("flat"); itr != options->shaderSets.end()) return itr->second;
    }

    auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp.vert", options);
    if (!vertexShader) vertexShader = assimp_vert(); // fallback to shaders/assimp_vert.cppp

    auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp_flat_shaded.frag", options);
    if (!fragmentShader) fragmentShader = assimp_flat_shaded_frag();

    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_TexCoord0", "", 2, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    shaderSet->addAttributeBinding("vsg_Color", "", 3, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));
    shaderSet->addAttributeBinding("vsg_position", "VSG_INSTANCE_POSITIONS", 3, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    shaderSet->addUniformBinding("displacementMap", "VSG_DISPLACEMENT_MAP", 0, 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("diffuseMap", "VSG_DIFFUSE_MAP", 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("material", "", 0, 10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::PhongMaterialValue::create());

    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    return shaderSet;
}

VSG_DECLSPEC ref_ptr<ShaderSet> vsg::createPhongShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("phong"); itr != options->shaderSets.end()) return itr->second;
    }

    auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp.vert", options);
    if (!vertexShader) vertexShader = assimp_vert(); // fallback to shaders/assimp_vert.cppp
    auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp_phong.frag", options);
    if (!fragmentShader) fragmentShader = assimp_phong_frag();

    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_TexCoord0", "", 2, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    shaderSet->addAttributeBinding("vsg_Color", "", 3, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));
    shaderSet->addAttributeBinding("vsg_position", "VSG_INSTANCE_POSITIONS", 4, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    shaderSet->addUniformBinding("displacementMap", "VSG_DISPLACEMENT_MAP", 0, 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("diffuseMap", "VSG_DIFFUSE_MAP", 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("normalMap", "VSG_NORMAL_MAP", 0, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec3Array2D::create(1,1));
    shaderSet->addUniformBinding("aoMap", "VSG_LIGHTMAP_MAP", 0, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("emissiveMap", "VSG_EMISSIVE_MAP", 0, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("material", "", 0, 10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::PhongMaterialValue::create());
    shaderSet->addUniformBinding("lightData", "VSG_VIEW_LIGHT_DATA", 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));

    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    return shaderSet;
}

VSG_DECLSPEC ref_ptr<ShaderSet> vsg::createPhysicsBasedRenderingShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("pbr"); itr != options->shaderSets.end()) return itr->second;
    }

    auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp.vert", options);
    if (!vertexShader) vertexShader = assimp_vert(); // fallback to shaders/assimp_vert.cppp

    auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/assimp_pbr.frag", options);
    if (!fragmentShader) fragmentShader = assimp_pbr_frag();

    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_TexCoord0", "", 2, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    shaderSet->addAttributeBinding("vsg_Color", "", 3, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));
    shaderSet->addAttributeBinding("vsg_position", "VSG_INSTANCE_POSITIONS", 3, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    shaderSet->addUniformBinding("displacementMap", "VSG_DISPLACEMENT_MAP", 0, 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("diffuseMap", "VSG_DIFFUSE_MAP", 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("mrMap", "VSG_METALLROUGHNESS_MAP", 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("normalMap", "VSG_NORMAL_MAP", 0, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec3Array2D::create(1,1));
    shaderSet->addUniformBinding("aoMap", "VSG_LIGHTMAP_MAP", 0, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("emissiveMap", "VSG_EMISSIVE_MAP", 0, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("specularMap", "VSG_SPECULAR_MAP", 0, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array2D::create(1,1));
    shaderSet->addUniformBinding("material", "", 0, 10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::PbrMaterialValue::create());
    shaderSet->addUniformBinding("lightData", "VSG_VIEW_LIGHT_DATA", 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));

    // additional defiines
    // VSG_GREYSACLE_DIFFUSE_MAP, VSG_TWOSIDED, VSG_WORKFLOW_SPECGLOSS

    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    return shaderSet;
}
