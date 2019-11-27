#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/Command.h>

namespace vsg
{
    class VSG_DECLSPEC RayTracingShaderGroup : public Inherit<Object, RayTracingShaderGroup>
    {
    public:
        RayTracingShaderGroup();

        void read(Input& input) override;
        void write(Output& output) const override;

        void applyTo(VkRayTracingShaderGroupCreateInfoNV& shaderGroupInfo) const;

        VkRayTracingShaderGroupTypeNV type = VK_RAY_TRACING_SHADER_GROUP_TYPE_MAX_ENUM_NV;
        uint32_t generalShader = VK_SHADER_UNUSED_NV;
        uint32_t closestHitShader = VK_SHADER_UNUSED_NV;
        uint32_t anyHitShader = VK_SHADER_UNUSED_NV;
        uint32_t intersectionShader = VK_SHADER_UNUSED_NV;

        BufferData bufferData;

    protected:
        virtual ~RayTracingShaderGroup();
    };
    VSG_type_name(vsg::RayTracingShaderGroup);

    using RayTracingShaderGroups = std::vector<ref_ptr<RayTracingShaderGroup>>;

} // namespace vsg
