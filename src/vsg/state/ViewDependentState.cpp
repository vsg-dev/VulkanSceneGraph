/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/state/ViewDependentState.h>

using namespace vsg;

//////////////////////////////////////
//
// ViewDescriptorSetLayout
//
ViewDescriptorSetLayout::ViewDescriptorSetLayout()
{
}

void ViewDescriptorSetLayout::read(Input& input)
{
    Object::read(input);
}

void ViewDescriptorSetLayout::write(Output& output) const
{
    Object::write(output);
}

void ViewDescriptorSetLayout::compile(Context& context)
{
    if (!_viewDescriptorSetLayout && context.viewDependentState && context.viewDependentState->descriptorSetLayout)
    {
        _viewDescriptorSetLayout = context.viewDependentState->descriptorSetLayout;
        _viewDescriptorSetLayout->compile(context);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BindViewDescriptorSets
//
BindViewDescriptorSets::BindViewDescriptorSets() :
    Inherit(2), // slot 2
    pipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
    firstSet(0)
{
}

void BindViewDescriptorSets::read(Input& input)
{
    _vulkanData.clear();

    StateCommand::read(input);

    input.readValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    input.read("layout", layout);
    input.read("firstSet", firstSet);
}

void BindViewDescriptorSets::write(Output& output) const
{
    StateCommand::write(output);

    output.writeValue<uint32_t>("pipelineBindPoint", pipelineBindPoint);
    output.write("layout", layout);
    output.write("firstSet", firstSet);
}

void BindViewDescriptorSets::compile(Context& context)
{
    layout->compile(context);
    context.viewDependentState->compile(context);
}

void BindViewDescriptorSets::record(CommandBuffer& commandBuffer) const
{
    commandBuffer.viewDependentState->bindDescriptorSets(commandBuffer, pipelineBindPoint, layout->vk(commandBuffer.deviceID), firstSet);
}

//////////////////////////////////////
//
// ViewDependentState
//
ViewDependentState::ViewDependentState(uint32_t maxNumberLights)
{
    lightData = vec4Array::create(maxNumberLights); // spot light requires 3 vec4's per light
}

ViewDependentState::~ViewDependentState()
{
}

void ViewDependentState::compile(Context& context)
{
    //std::cout<<"ViewDependentState::compile()"<<std::endl;
    if (!bufferedDescriptors.empty()) return;

    DescriptorSetLayoutBindings descriptorBindings{
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);
    descriptorSetLayout->compile(context);

    int numBuffers = 3;
    for (int i = 0; i < numBuffers; ++i)
    {
        auto lightDescriptor = vsg::DescriptorBuffer::create(lightData, 0); // hardwired position for now
        auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{lightDescriptor});
        descriptorSet->compile(context);

        bufferedDescriptors.push_back(DescriptorPair{lightDescriptor, descriptorSet});
    }
}

void ViewDependentState::clear()
{
    // advance index
    bufferIndex = (bufferIndex + 1) % bufferedDescriptors.size();

    //std::cout<<"ViewDependentState::clear() bufferIndex = "<<bufferIndex<<std::endl;

    // clear data
    ambientLights.clear();
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();
}

void ViewDependentState::pack()
{
    // std::cout<<"ViewDependentState::pack() abmient "<<ambientLights.size()<<", diffuse "<<directionalLights.size()<<", point "<<pointLights.size()<<", spot "<<spotLights.size()<<std::endl;

    auto light_itr = lightData->begin();

    (*light_itr++) = vec4(static_cast<float>(ambientLights.size()),
                          static_cast<float>(directionalLights.size()),
                          static_cast<float>(pointLights.size()),
                          static_cast<float>(spotLights.size()));

    for (auto& [mv, light] : ambientLights)
    {
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
    }

    for (auto& [mv, light] : directionalLights)
    {
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), 0.0f);
    }

    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), 0.0f);
    }

    for (auto& [mv, light] : spotLights)
    {
        auto eye_position = mv * light->position;
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        float cos_innerAngle = static_cast<float>(cos(light->innerAngle));
        float cos_outerAngle = static_cast<float>(cos(light->outerAngle));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), cos_innerAngle);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), cos_outerAngle);
    }
#if 0
    for(auto itr = lightData->begin(); itr != light_itr; ++itr)
    {
        std::cout<<"   "<<*itr<<std::endl;
    }
#endif
}

void ViewDependentState::copy()
{
    //    std::cout<<"ViewDependentState::copy()"<<std::endl;

    auto& descriptorData = bufferedDescriptors[bufferIndex];
    for (auto& bufferInfo : descriptorData.lightDescriptor->bufferInfoList)
    {
        bufferInfo->copyDataToBuffer();
    }
}

void ViewDependentState::bindDescriptorSets(CommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet)
{
    auto vk = bufferedDescriptors[bufferIndex].descriptorSet->vk(commandBuffer.deviceID);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, 1, &vk, 0, nullptr);
}
