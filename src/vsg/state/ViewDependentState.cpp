/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/vk/Context.h>
#include <vsg/io/write.h>

using namespace vsg;

//////////////////////////////////////
//
// ViewDescriptorSetLayout
//
ViewDescriptorSetLayout::ViewDescriptorSetLayout()
{
}

int ViewDescriptorSetLayout::compare(const Object& rhs_object) const
{
    int result = DescriptorSetLayout::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer(_viewDescriptorSetLayout, rhs._viewDescriptorSetLayout);
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

int BindViewDescriptorSets::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(pipelineBindPoint, rhs.pipelineBindPoint))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    return compare_value(firstSet, rhs.firstSet);
}

void BindViewDescriptorSets::read(Input& input)
{
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
    if (context.viewDependentState) context.viewDependentState->compile(context);
}

void BindViewDescriptorSets::record(CommandBuffer& commandBuffer) const
{
    commandBuffer.viewDependentState->bindDescriptorSets(commandBuffer, pipelineBindPoint, layout->vk(commandBuffer.deviceID), firstSet);
}

//////////////////////////////////////
//
// ViewDependentState
//
ViewDependentState::ViewDependentState(uint32_t maxNumberLights, uint32_t maxViewports)
{
    init(maxNumberLights, maxViewports);
}

ViewDependentState::~ViewDependentState()
{
}

void ViewDependentState::init(uint32_t maxNumberLights, uint32_t maxViewports)
{
    // info("ViewDependentState::init(", maxNumberLights, ", ", maxViewports, ") ", this);

    lightData = vec4Array::create(maxNumberLights);
    lightData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    lightDataBufferInfo = BufferInfo::create(lightData.get());

    viewportData = vec4Array::create(maxViewports);
    viewportData->properties.dataVariance = DYNAMIC_DATA_TRANSFER_AFTER_RECORD;
    viewportDataBufferInfo = BufferInfo::create(viewportData.get());

    descriptor = DescriptorBuffer::create(BufferInfoList{lightDataBufferInfo, viewportDataBufferInfo}, 0); // hardwired position for now

    auto shadwoMapSampler = vsg::Sampler::create();
    shadwoMapSampler->minFilter = VK_FILTER_NEAREST;
    shadwoMapSampler->magFilter = VK_FILTER_NEAREST;
    shadwoMapSampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadwoMapSampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadwoMapSampler->anisotropyEnable = VK_FALSE;
    shadwoMapSampler->maxAnisotropy = 1;
    shadwoMapSampler->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    // image->imageType = VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D?
    // imageView->viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY
    shadowMapData = floatArray3D::create(2048, 2048, 8, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT});
    shadowMaps = DescriptorImage::create(shadwoMapSampler, shadowMapData, 2);


    DescriptorSetLayoutBindings descriptorBindings{
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // lightData
        VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // viewportData
        VkDescriptorSetLayoutBinding{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };

    descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);
    descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{descriptor, shadowMaps});

    for(uint32_t k = 0; k < shadowMapData->depth(); ++k)
    {
        for(uint32_t j = 0; j < shadowMapData->height(); ++j)
        {
            float* data = shadowMapData->data(shadowMapData->index(0, j, k));
            for(uint32_t i = 0; i < shadowMapData->width(); ++i)
            {
                *(data++) = sin(vsg::PI * static_cast<double>(i)/static_cast<double>(shadowMapData->width()-1));
            }
        }
    }

    //vsg::write(shadowMapData, "test.vsgt");

}

void ViewDependentState::compile(Context& context)
{
    info("ViewDependentState::compile()", this);

    descriptorSet->compile(context);
}

void ViewDependentState::clear()
{
    //debug("ViewDependentState::clear() bufferIndex = ", bufferIndex);

    // clear data
    ambientLights.clear();
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();
}

void ViewDependentState::traverse(RecordTraversal& rt, const View& view)
{
#if 1
    // usefule reference : https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
    // PCF filtering : https://github.com/SaschaWillems/Vulkan/issues/231
    // sampler2DArrayShadow
    // https://ogldev.org/www/tutorial42/tutorial42.html

    info("\n\nViewDependentState::traverse(", &rt, ", ", &view, ")");
    for (auto& [mv, light] : directionalLights)
    {
        if (light->shadowMaps == 0) continue;


        // compute directional light space
        auto projectionMatrix = view.camera->projectionMatrix->transform();
        auto viewMatrix = view.camera->viewMatrix->transform();

        // view direction in world coords
        auto view_direction = normalize(dvec3(0.0, 0.0, -1.0) * (projectionMatrix * viewMatrix));
        auto view_up = normalize(dvec3(0.0, -1.0, 0.0) * (projectionMatrix * viewMatrix));

        // light direction in world coords
        auto light_direction = normalize(light->direction * (inverse_3x3(mv * inverse(viewMatrix))));


        info("   directional light : light direction in world = ", light_direction, ", light->shadowMaps = ", light->shadowMaps);
        info("      light->direction in model = ", light->direction);
        info("      view_direction in world = ", view_direction);
        info("      view_up in world = ", view_up);

        auto light_x_direction = cross(light_direction, view_direction);
        auto light_x_up = cross(light_direction, view_up);

        auto light_x = (length(light_x_direction) > length(light_x_up)) ? normalize(light_x_direction) : normalize(light_x_up);
        auto light_y = cross(light_x, light_direction);
        auto light_z = light_direction;


        info("     light_x = ", light_x);
        info("     light_y = ", light_y);
        info("     light_z = ", light_z);

        auto eye_point = inverse(viewMatrix) * dvec3(0.0, 0.0, 0.0);

        auto clipToEye = inverse(projectionMatrix);

        auto n = -(clipToEye * dvec3(0.0, 0.0, 1.0)).z;
        auto f = -(clipToEye * dvec3(0.0, 0.0, 0.0)).z;

#if 0
        // clamp the near and far values
        double maxShadowDistance = 1000.0;
        if (n > maxShadowDistance)
        {
            info("Oopps near is further than the maxShadowDistance!");
            n = maxShadowDistance * 0.5;
            f = maxShadowDistance;
        }
        if (f > maxShadowDistance)
        {
            f = maxShadowDistance;
        }
#endif
        double range = f-n;
        info("  n = ", n, ", f = ", f, ", range = ", range);

        auto computeLightSpaceBounds = [&](double n, double f)
        {
            auto clipToWorld = inverse(projectionMatrix * viewMatrix);

            std::vector<dvec3> corners;
            corners.reserve(8);
            corners.push_back(clipToWorld * dvec3(-1.0, -1.0, n));
            corners.push_back(clipToWorld * dvec3(-1.0, 1.0, n));
            corners.push_back(clipToWorld * dvec3(1.0, -1.0, n));
            corners.push_back(clipToWorld * dvec3(1.0, 1.0, n));
            corners.push_back(clipToWorld * dvec3(-1.0, -1.0, f));
            corners.push_back(clipToWorld * dvec3(-1.0, 1.0, f));
            corners.push_back(clipToWorld * dvec3(1.0, -1.0, f));
            corners.push_back(clipToWorld * dvec3(1.0, 1.0, f));

            dbox lightSpaceFrustumBounds;
            for(auto& v : corners)
            {
                lightSpaceFrustumBounds.add(dot(v, light_x), dot(v, light_y), dot(v, light_z));
            }

            info("    light_x = ", light_x);
            info("    light_y = ", light_y);
            info("    light_z = ", light_z);
            info("    lightSpaceFrustumBounds = ", lightSpaceFrustumBounds);
            info("    eye_point = ", eye_point);
            for(auto& v : corners)
            {
                info("      ", v);
            }
        };


        if (light->shadowMaps > 1)
        {
            auto Clog = [](double n, double f, double i, double m) -> double
            {
                return n * std::pow((f/n), (i/m));
            };

            auto Cuniform = [](double n, double f, double i, double m) -> double
            {
                return n + (f - n) * (i/m);
            };

            auto Cpractical = [&Clog, &Cuniform](double n, double f, double i, double m, double lambda) -> double
            {
                return Clog(n, f, i, m) * lambda + Cuniform(n, f, i, m) * (1.0-lambda);
            };

            double i = 0.0;
            double m = static_cast<double>(light->shadowMaps);
            double delta = 1.0;
            double lambda = 0.5;
            for(uint32_t si = 0; si < light->shadowMaps; ++si)
            {
                // computeLightSpaceBounds();

                info("    Clog() = ", Clog(n, f, i, m), ", ", Clog(n, f, i+delta, m));
                info("    Cuniform() = ", Cuniform(n, f, i, m), ", ", Cuniform(n, f, i+delta, m));
                info("    Cpractical(", n, ", ", f, ", ", i, ", ", m, ", ", lambda,") = ", Cpractical(n, f, i, m, lambda), ", ", Cpractical(n, f, i+delta, m, lambda));

                dvec3 eye_near(0.0, 0.0, -Cpractical(n, f, i, m, lambda));
                dvec3 eye_far(0.0, 0.0, -Cpractical(n, f, i+delta, m, lambda));

                auto clip_near = projectionMatrix * eye_near;
                auto clip_far = projectionMatrix * eye_far;

                info("    clip_near = ", clip_near);
                info("    clip_far = ", clip_far);

                computeLightSpaceBounds(clip_near.z, clip_far.z);

                i += 1.0;
            }
        }
        else
        {
            computeLightSpaceBounds(1.0, 0.0);
        }


    }
#endif

#if 1
    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        info("   positional light : position = ", eye_position, ", light->shadowMaps = ", light->shadowMaps);
    }

    for (auto& [mv, light] : spotLights)
    {
        auto eye_position =  mv * light->position;
        auto eye_direction1 = normalize(light->direction * inverse_3x3(mv));
        info("   spot light : light->direction = ", light->direction, ", position = ", eye_position, ", direction = ", eye_direction1, ", light->shadowMaps = ", light->shadowMaps);
    }
#endif
}

void ViewDependentState::pack()
{
    //debug("ViewDependentState::pack() ambient ", ambientLights.size(), ", diffuse ", directionalLights.size(), ", point ", pointLights.size(), ", spot ", spotLights.size());

    auto light_itr = lightData->begin();
    lightData->dirty();

    (*light_itr++) = vec4(static_cast<float>(ambientLights.size()),
                          static_cast<float>(directionalLights.size()),
                          static_cast<float>(pointLights.size()),
                          static_cast<float>(spotLights.size()));

    // lightData requirements = vec4 * (num_ambientLights + 3 * num_directionLights + 3 * num_pointLights + 4 * num_spotLights + 4 * num_shadow_maps)

    for (auto& entry : ambientLights)
    {
        auto light = entry.second;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
    }

    for (auto& [mv, light] : directionalLights)
    {
        auto eye_direction = normalize(inverse_3x3(mv) * light->direction);
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_direction.x), static_cast<float>(eye_direction.y), static_cast<float>(eye_direction.z), 0.0f);
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
    }

    for (auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(static_cast<float>(eye_position.x), static_cast<float>(eye_position.y), static_cast<float>(eye_position.z), 0.0f);
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
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
        (*light_itr++).set(static_cast<float>(light->shadowMaps), 0.0f, 0.0f, 0.0f); // shadow map setting
    }
#if 0
    for(auto itr = lightData->begin(); itr != light_itr; ++itr)
    {
        debug("   ", *itr);
    }
#endif
}

void ViewDependentState::bindDescriptorSets(CommandBuffer& commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet)
{
    auto vk = descriptorSet->vk(commandBuffer.deviceID);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, 1, &vk, 0, nullptr);
}
