#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Value.h>
#include <vsg/maths/mat4.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/vk/DeviceMemory.h>

namespace vsg
{

    /// AccelerationStructure is a base class for top/bottom level acceleration structure classes.
    class VSG_DECLSPEC AccelerationStructure : public Inherit<Object, AccelerationStructure>
    {
    public:
        AccelerationStructure(VkAccelerationStructureTypeKHR type, Device* device);

        virtual void compile(Context& context);

        operator VkAccelerationStructureKHR() const { return _accelerationStructure; }
        operator VkAccelerationStructureBuildGeometryInfoKHR() const { return _accelerationStructureBuildGeometryInfo; }

        uint64_t handle() const { return _handle; }

        VkDeviceSize requiredScratchSize() const { return _requiredBuildScratchSize; }

    protected:
        virtual ~AccelerationStructure();

        VkAccelerationStructureKHR _accelerationStructure;
        VkAccelerationStructureCreateInfoKHR _accelerationStructureInfo;
        std::vector<uint32_t> _geometryPrimitiveCounts;
        VkAccelerationStructureBuildGeometryInfoKHR _accelerationStructureBuildGeometryInfo;
        ref_ptr<Buffer> _buffer;
        ref_ptr<DeviceMemory> _memory;
        uint64_t _handle = 0;
        VkDeviceSize _requiredBuildScratchSize;

        ref_ptr<Device> _device;
    };
    VSG_type_name(vsg::AccelerationStructure);

    using AccelerationStructures = std::vector<ref_ptr<AccelerationStructure>>;

} // namespace vsg
