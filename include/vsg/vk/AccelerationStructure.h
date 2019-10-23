#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Value.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/maths/mat4.h>

namespace vsg
{
    class VSG_DECLSPEC AccelerationGeometry : public Inherit<Object, AccelerationGeometry>
    {
    public:
        AccelerationGeometry(Allocator* allocator = nullptr);

        void compile(Context& context);

        operator VkGeometryNV() const { return _geometry; }

        DataList _arrays;
        ref_ptr<Data> _indices;

        // compiled data
        BufferData _vertexBuffer;
        BufferData _indexBuffer;
        VkGeometryNV _geometry;
    };
    using AccelerationGeometries = std::vector<ref_ptr<AccelerationGeometry>>;

    struct GeometryInstance
    {
        float transform[12]; // transform matrix
        uint32_t instanceId : 24; // id available in shader as gl_InstanceCustomIndexNV in shader
        uint32_t mask : 8; // mask used with rayMask  for culling
        uint32_t instanceOffset : 24; // offset into shader binding table for shaders
        uint32_t flags : 8; // VkGeometryInstanceFlagBitsNV 
        uint64_t accelerationStructureHandle; // handle to bottomlevel acceleration structure
    };
    VSG_value(GeometryInstanceValue, GeometryInstance);

    class VSG_DECLSPEC AccelerationStructure : public Inherit<Command, AccelerationStructure>
    {
    public:
        AccelerationStructure(VkAccelerationStructureTypeNV type, Device* device, Allocator* allocator = nullptr);

        void compile(Context& context) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        operator VkAccelerationStructureNV() const { return _accelerationStructure; }

        const uint64_t handle() const { return _handle; }

        const VkDeviceSize requiredScratchSize() const { return _requiredBuildScratchSize; }

    protected:
        virtual ~AccelerationStructure();

        VkAccelerationStructureNV _accelerationStructure;
        VkAccelerationStructureInfoNV _accelerationStructureInfo;
        ref_ptr<DeviceMemory> _memory;
        uint64_t _handle;
        VkDeviceSize _requiredBuildScratchSize;

        ref_ptr<Device> _device;
    };

    class VSG_DECLSPEC BottomLevelAccelerationStructure : public Inherit<AccelerationStructure, BottomLevelAccelerationStructure>
    {
    public:
        BottomLevelAccelerationStructure(Device* device, Allocator* allocator = nullptr);

        void compile(Context& context) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        AccelerationGeometries _geometries;

        // compiled data
        std::vector<VkGeometryNV> _vkGeometries;
    };

    
    class VSG_DECLSPEC TopLevelAccelerationStructure : public Inherit<AccelerationStructure, TopLevelAccelerationStructure>
    {
    public:
        TopLevelAccelerationStructure(Device* device, Allocator* allocator = nullptr);

        void compile(Context& context) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        dmat4 _transform;
        ref_ptr<BottomLevelAccelerationStructure> _instanceSource;

        // compiled data
        ref_ptr<GeometryInstanceValue> _instance;
        ref_ptr<Buffer> _instanceBuffer;
    };

    using AccelerationStructures = std::vector<ref_ptr<AccelerationStructure>>;

    class VSG_DECLSPEC DescriptorAccelerationStructure : public Inherit<Descriptor, DescriptorAccelerationStructure>
    {
    public:
        DescriptorAccelerationStructure();

        DescriptorAccelerationStructure(const AccelerationStructures& accelerationStructures, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);

        AccelerationStructures& getAccelerationStructures() { return _accelerationStructures; }
        const AccelerationStructures& getAccelerationStructures() const { return _accelerationStructures; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        uint32_t getNumDescriptors() const override;

    protected:
        AccelerationStructures _accelerationStructures;

        // populated by compile()
        std::vector <VkAccelerationStructureNV> _vkAccelerationStructures;
        VkWriteDescriptorSetAccelerationStructureNV _descriptorAccelerationStructureInfo;
    };
    VSG_type_name(vsg::DescriptorAccelerationStructure)

} // namespace vsg
