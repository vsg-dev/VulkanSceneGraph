#pragma once

#include <vsg/core/Object.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{
    using Names = std::vector<const char*>;

    extern Names validateInstancelayerNames(const Names& names);

    inline VkQueue createDeviceQueue(VkDevice device, int graphicsFamily)
    {
        VkQueue queue;
        vkGetDeviceQueue(device, graphicsFamily, 0, &queue);
        return queue;
    }

    class Instance : public vsg::Object
    {
    public:
        Instance(VkInstance instance, VkAllocationCallbacks* pAllocator=nullptr) : _instance(instance), _pAllocator(pAllocator) {}

        Instance(Names& instanceExtensions, Names& layers, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkInstance() const { return _instance; }

    protected:

        virtual ~Instance();

        VkInstance              _instance;
        VkAllocationCallbacks*  _pAllocator;
    };
}
