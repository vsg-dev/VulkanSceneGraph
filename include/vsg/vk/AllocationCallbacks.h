#pragma once

#include <vsg/core/Object.h>

#include <vulkan/vulkan.h>

namespace vsg
{
    class AllocationCallbacks : public vsg::Object, public VkAllocationCallbacks
    {
    public:
        AllocationCallbacks() :
            VkAllocationCallbacks{} {}

        operator VkAllocationCallbacks* () { return this!=nullptr ? &static_cast<VkAllocationCallbacks&>(*this) : nullptr; }

    protected:

        virtual ~AllocationCallbacks() {}
    };
}
