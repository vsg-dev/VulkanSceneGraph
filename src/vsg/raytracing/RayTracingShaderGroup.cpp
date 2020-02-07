/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/RayTracingShaderGroup.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// RayTracingShaderGroup
//
RayTracingShaderGroup::RayTracingShaderGroup()
{
}

RayTracingShaderGroup::~RayTracingShaderGroup()
{
}

void RayTracingShaderGroup::read(Input& input)
{
    Object::read(input);
}

void RayTracingShaderGroup::write(Output& output) const
{
    Object::write(output);
}

void RayTracingShaderGroup::applyTo(VkRayTracingShaderGroupCreateInfoNV& shaderGroupInfo) const
{
    shaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
    shaderGroupInfo.pNext = nullptr;
    shaderGroupInfo.type = type;
    shaderGroupInfo.generalShader = generalShader;
    shaderGroupInfo.closestHitShader = closestHitShader;
    shaderGroupInfo.anyHitShader = anyHitShader;
    shaderGroupInfo.intersectionShader = intersectionShader;
}
