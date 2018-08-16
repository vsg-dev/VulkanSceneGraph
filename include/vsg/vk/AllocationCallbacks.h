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

    protected:

        virtual ~AllocationCallbacks() {}
    };
}
