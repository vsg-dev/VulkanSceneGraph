#pragma once

#include <vsg/vk/Device.h>

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

    class ImageMemoryBarrier : public Object, public VkImageMemoryBarrier
    {
    public:

        ImageMemoryBarrier(VkAccessFlags in_srcAccessMask=0, VkAccessFlags in_destAccessMask=0,
                            VkImageLayout in_oldLayout=VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout in_newLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                            Image* in_image=nullptr);

        void cmdPiplineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

    //protected:
        virtual ~ImageMemoryBarrier();
    };

}
