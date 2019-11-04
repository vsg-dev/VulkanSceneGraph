#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>
#include <vsg/vk/ShaderStage.h>

namespace vsg
{
    using RayTracingShaderGroupCreateInfos = std::vector<VkRayTracingShaderGroupCreateInfoNV>;

    class VSG_DECLSPEC RayTracingShaderBindings : public Inherit<Object, RayTracingShaderBindings>
    {
    public:
        RayTracingShaderBindings();

        RayTracingShaderBindings(const ShaderStages& shaders, Device* device, AllocationCallbacks* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context, const VkPipeline& pipeline);

        const RayTracingShaderGroupCreateInfos& createInfos() const { return _shaderGroupCreateInfos; }

        VkBuffer raygenTableBuffer() const;
        VkDeviceSize raygeTableOffset() const;

        VkBuffer missTableBuffer() const;
        VkDeviceSize missTableOffset() const;
        VkDeviceSize missTableStride() const;

        VkBuffer hitTableBuffer() const;
        VkDeviceSize hitTableOffset() const;
        VkDeviceSize hitTableStride() const;

        VkBuffer callableTableBuffer() const;
        VkDeviceSize callableTableOffset() const;
        VkDeviceSize callableTableStride() const;

        Device* device() const { return _device; }

    protected:
        virtual ~RayTracingShaderBindings();

        VkDeviceSize copyShaderIdentifier(uint8_t* data, const uint8_t* shaderHandleStorage, uint32_t groupIndex, uint32_t shaderGroupHandleSize);

        ref_ptr<Device> _device;
        VkPhysicalDeviceRayTracingPropertiesNV _rayTracingProperties;
        ShaderStages _shaderStages;
        RayTracingShaderGroupCreateInfos _shaderGroupCreateInfos;

        // not sure what to do with these yet
        uint32_t _raygenIndex;
        uint32_t _missIndex;
        uint32_t _closestHitIndex;
        uint32_t _callableIndex;

        ref_ptr<Buffer> _bindingTableBuffer;
        ref_ptr<DeviceMemory> _bindingTableMemory;
    };
    VSG_type_name(vsg::RayTracingShaderBindings);

    

} // namespace vsg
