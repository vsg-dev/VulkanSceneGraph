#pragma once

#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class Image : public Object
    {
    public:
        Image(VkImage Image, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Image, VkResult, VK_SUCCESS>;
        static Result create(Device* device, const VkImageCreateInfo& createImageInfo, AllocationCallbacks* allocator=nullptr);

        VkImage image() const { return _image; }

        operator VkImage () const { return _image; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
        {
            VkResult result = vkBindImageMemory(*_device, _image, *deviceMemory, memoryOffset);
            if (result == VK_SUCCESS)
            {
                _deviceMemory = deviceMemory;
                _memoryOffset = memoryOffset;
            }
            return result;
        }

    protected:
        virtual ~Image();

        VkImage                         _image;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;

        ref_ptr<DeviceMemory>           _deviceMemory;
        VkDeviceSize                    _memoryOffset;
    };

    class ImageMemoryBarrier : public Object, public VkImageMemoryBarrier
    {
    public:

        ImageMemoryBarrier(VkAccessFlags in_srcAccessMask=0, VkAccessFlags in_destAccessMask=0,
                            VkImageLayout in_oldLayout=VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout in_newLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                            Image* in_image=nullptr);

        void cmdPiplineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

        virtual ~ImageMemoryBarrier();

    protected:
        ref_ptr<Image>  _image;
    };

}
