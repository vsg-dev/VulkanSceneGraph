#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Command.h>

#include <iostream>

namespace vsg
{

    struct ScratchBuffer
    {
        ScratchBuffer(uint32_t size);
        ScratchBuffer(const ScratchBuffer& parent, uint32_t minimumSize);

        ~ScratchBuffer();

        template<typename T>
        T* allocate(uint32_t count = 1)
        {
            if (count == 0) return nullptr;

            T* ptr = reinterpret_cast<T*>(base_ptr);
            base_ptr += sizeof(T) * count;
            return ptr;
        }

        uint8_t* buffer_begin = nullptr;
        uint8_t* buffer_end = nullptr;
        uint8_t* base_ptr = nullptr;
        bool requiresDelete = false;
    };

    struct VulkanInfo : public Inherit<Object, VulkanInfo>
    {
        ref_ptr<VulkanInfo> next;

        virtual uint32_t infoSize() const = 0;
        virtual void* assign(ScratchBuffer& buffer) const = 0;
    };

    struct MemoryBarrier : public Inherit<Object, MemoryBarrier>
    {
        ref_ptr<VulkanInfo> next;
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;

        uint32_t infoSize() const { return sizeof(VkMemoryBarrier) + (next ? next->infoSize() : 0); }
        void assign(VkMemoryBarrier& info, ScratchBuffer& buffer) const;
    };

    struct BufferMemoryBarrier : public Inherit<Object, BufferMemoryBarrier>
    {
        ref_ptr<VulkanInfo> next;
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;
        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
        ref_ptr<Buffer> buffer;
        VkDeviceSize offset = 0;
        VkDeviceSize size = 0;

        uint32_t infoSize() const { return sizeof(VkBufferMemoryBarrier) + (next ? next->infoSize() : 0); }
        void assign(VkBufferMemoryBarrier& info, ScratchBuffer& buffer) const;
    };

    struct ImageMemoryBarrier : public Inherit<Object, ImageMemoryBarrier>
    {
        ImageMemoryBarrier(VkAccessFlags in_srcAccessMask = 0,
                           VkAccessFlags in_dstAccessMask = 0,
                           VkImageLayout in_oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                           VkImageLayout in_newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                           uint32_t in_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                           uint32_t in_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                           ref_ptr<Image> in_image = {},
                           VkImageSubresourceRange in_subresourceRange = {0, 0, 0, 0, 0}) :
            srcAccessMask(in_srcAccessMask),
            dstAccessMask(in_dstAccessMask),
            oldLayout(in_oldLayout),
            newLayout(in_newLayout),
            srcQueueFamilyIndex(in_srcQueueFamilyIndex),
            dstQueueFamilyIndex(in_dstQueueFamilyIndex),
            image(in_image),
            subresourceRange(in_subresourceRange) {}

        ref_ptr<VulkanInfo> next;
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;
        VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ref_ptr<Image> image;
        VkImageSubresourceRange subresourceRange = {0, 0, 0, 0, 0};

        uint32_t infoSize() const { return sizeof(VkImageMemoryBarrier) + (next ? next->infoSize() : 0); }
        void assign(VkImageMemoryBarrier& info, ScratchBuffer& buffer) const;
    };

    struct SampleLocations : public Inherit<VulkanInfo, SampleLocations>
    {
        ref_ptr<VulkanInfo> next;
        VkSampleCountFlagBits sampleLocationsPerPixel = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        VkExtent2D sampleLocationGridSize = {0, 0};
        std::vector<vec2> sampleLocations;

        uint32_t infoSize() const override { return sizeof(VkSampleLocationsInfoEXT) + (next ? next->infoSize() : 0); }
        void* assign(ScratchBuffer& buffer) const override;
    };

    class VSG_DECLSPEC PipelineBarrier : public Inherit<Command, PipelineBarrier>
    {
    public:
        PipelineBarrier();

        template<class T>
        PipelineBarrier(VkPipelineStageFlags in_srcStageMask, VkPipelineStageFlags in_destStageMask, VkDependencyFlags in_dependencyFlags, T barrier) :
            srcStageMask(in_srcStageMask),
            dstStageMask(in_destStageMask),
            dependencyFlags(in_dependencyFlags)
        {
            add(barrier);
        }

        void dispatch(CommandBuffer& commandBuffer) const override;

        void add(ref_ptr<MemoryBarrier> mb) { memoryBarriers.emplace_back(mb); }
        void add(ref_ptr<BufferMemoryBarrier> bmb) { bufferMemoryBarriers.emplace_back(bmb); }
        void add(ref_ptr<ImageMemoryBarrier> imb) { imageMemoryBarriers.emplace_back(imb); }

        using MemoryBarriers = std::vector<ref_ptr<MemoryBarrier>>;
        using BufferMemoryBarriers = std::vector<ref_ptr<BufferMemoryBarrier>>;
        using ImageMemoryBarriers = std::vector<ref_ptr<ImageMemoryBarrier>>;

        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkDependencyFlags dependencyFlags;

        MemoryBarriers memoryBarriers;
        BufferMemoryBarriers bufferMemoryBarriers;
        ImageMemoryBarriers imageMemoryBarriers;

    protected:
        virtual ~PipelineBarrier();
    };
    VSG_type_name(vsg::PipelineBarrier);

} // namespace vsg
