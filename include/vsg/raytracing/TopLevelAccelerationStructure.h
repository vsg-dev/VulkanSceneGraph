#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Value.h>
#include <vsg/core/Array.h>
#include <vsg/raytracing/AccelerationStructure.h>
#include <vsg/raytracing/BottomLevelAccelerationStructure.h>

namespace vsg
{
    // this structure is required to populate the top level structures instance buffer
    struct VkGeometryInstance
    {
        float transform[12]; // transform matrix
        uint32_t instanceId : 24; // id available in shader as gl_InstanceCustomIndexNV in shader
        uint32_t mask : 8; // mask used with rayMask for culling
        uint32_t instanceOffset : 24; // offset into shader binding table for shaders
        uint32_t flags : 8; // VkGeometryInstanceFlagBitsNV 
        uint64_t accelerationStructureHandle; // handle to bottomlevel acceleration structure
    };
    VSG_value(VkGeometryInstanceValue, VkGeometryInstance);
    VSG_array(VkGeometryInstanceArray, VkGeometryInstance);

    // An instance of a bottom level acceleration structure referenc by a top level acceleration structure
    class VSG_DECLSPEC GeometryInstance : public Inherit<Object, GeometryInstance>
    {
    public:
        GeometryInstance();

        operator VkGeometryInstance() const
        {
            VkGeometryInstance inst;
            inst.instanceId = _id;
            inst.mask = _mask;
            inst.instanceOffset = _shaderOffset;
            inst.flags = _flags;
            inst.accelerationStructureHandle = _accelerationStructure->handle();
            for (unsigned int i = 0; i < 12; i++)
                inst.transform[i] = (float)_transform.data()[i];
            return inst;
        }

        mat4 _transform;
        uint32_t _id;
        uint32_t _mask;
        uint32_t _shaderOffset;
        uint32_t _flags;
        ref_ptr<BottomLevelAccelerationStructure> _accelerationStructure;
    };
    using GeometryInstances = std::vector<ref_ptr<GeometryInstance>>;

    class VSG_DECLSPEC TopLevelAccelerationStructure : public Inherit<AccelerationStructure, TopLevelAccelerationStructure>
    {
    public:
        TopLevelAccelerationStructure(Device* device, Allocator* allocator = nullptr);

        void compile(Context& context) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        GeometryInstances _geometries;

        // compiled data
        ref_ptr<VkGeometryInstanceArray> _instances;
        ref_ptr<Buffer> _instanceBuffer;
    };
} // namespace vsg
