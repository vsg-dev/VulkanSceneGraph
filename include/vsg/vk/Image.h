#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{
    class Image : public Object
    {
    public:
        Image(Device* device, VkImage Image, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Image, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkImageCreateInfo& createImageInfo, AllocationCallbacks* allocator=nullptr);

        VkImage image() const { return _image; }

        operator VkImage () const { return _image; }

    protected:
        virtual ~Image();

        ref_ptr<Device>                 _device;
        VkImage                         _image;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
