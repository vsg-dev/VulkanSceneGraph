#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Command.h>
#include <vsg/core/ScratchMemory.h>
#include <vsg/state/Buffer.h>
#include <vsg/state/Image.h>

namespace vsg
{

    struct VSG_DECLSPEC VulkanInfo : public Inherit<Object, VulkanInfo>
    {
        ref_ptr<VulkanInfo> next;

        virtual void* assign(CommandBuffer& commandBuffer) const = 0;
    };

    struct VSG_DECLSPEC MemoryBarrier : public Inherit<Object, MemoryBarrier>
    {
        MemoryBarrier(VkAccessFlags in_srcAccessMask = 0,
                      VkAccessFlags in_dstAccessMask = 0) :
            srcAccessMask(in_srcAccessMask),
            dstAccessMask(in_dstAccessMask) {}

        ref_ptr<VulkanInfo> next;
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;

        void assign(CommandBuffer& commandBuffer, VkMemoryBarrier& info) const;
    };
    VSG_type_name(vsg::MemoryBarrier);
    using MemoryBarriers = std::vector<ref_ptr<MemoryBarrier>>;

    struct VSG_DECLSPEC BufferMemoryBarrier : public Inherit<Object, BufferMemoryBarrier>
    {
        BufferMemoryBarrier(VkAccessFlags in_srcAccessMask = 0,
                            VkAccessFlags in_dstAccessMask = 0,
                            uint32_t in_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            uint32_t in_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            ref_ptr<Buffer> in_buffer = {},
                            VkDeviceSize in_offset = 0,
                            VkDeviceSize in_size = 0) :
            srcAccessMask(in_srcAccessMask),
            dstAccessMask(in_dstAccessMask),
            srcQueueFamilyIndex(in_srcQueueFamilyIndex),
            dstQueueFamilyIndex(in_dstQueueFamilyIndex),
            buffer(in_buffer),
            offset(in_offset),
            size(in_size) {}

        ref_ptr<VulkanInfo> next;
        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;
        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
        ref_ptr<Buffer> buffer;
        VkDeviceSize offset = 0;
        VkDeviceSize size = 0;

        void assign(CommandBuffer& commandBuffer, VkBufferMemoryBarrier& info) const;
    };
    VSG_type_name(vsg::BufferMemoryBarrier);
    using BufferMemoryBarriers = std::vector<ref_ptr<BufferMemoryBarrier>>;

    struct VSG_DECLSPEC ImageMemoryBarrier : public Inherit<Object, ImageMemoryBarrier>
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
        VkAccessFlags dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ref_ptr<Image> image;
        VkImageSubresourceRange subresourceRange = {0, 0, 0, 0, 0};

        void assign(CommandBuffer& commandBuffer, VkImageMemoryBarrier& info) const;
    };
    VSG_type_name(vsg::ImageMemoryBarrier);
    using ImageMemoryBarriers = std::vector<ref_ptr<ImageMemoryBarrier>>;

    // TODO decide where SampleLocations belongs, possibly RenderPass?
    struct VSG_DECLSPEC SampleLocations : public Inherit<VulkanInfo, SampleLocations>
    {
        ref_ptr<VulkanInfo> next;
        VkSampleCountFlagBits sampleLocationsPerPixel = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        VkExtent2D sampleLocationGridSize = {0, 0};
        std::vector<vec2> sampleLocations;

        void* assign(CommandBuffer& commandBuffer) const override;
    };

    class VSG_DECLSPEC PipelineBarrier : public Inherit<Command, PipelineBarrier>
    {
    public:
        PipelineBarrier();

        template<typename... Args>
        PipelineBarrier(VkPipelineStageFlags in_srcStageMask, VkPipelineStageFlags in_destStageMask, VkDependencyFlags in_dependencyFlags, Args&&... args) :
            srcStageMask(in_srcStageMask),
            dstStageMask(in_destStageMask),
            dependencyFlags(in_dependencyFlags)
        {
            (add(args), ...);
        }

        void record(CommandBuffer& commandBuffer) const override;

        void add(ref_ptr<MemoryBarrier> mb) { memoryBarriers.emplace_back(mb); }
        void add(ref_ptr<BufferMemoryBarrier> bmb) { bufferMemoryBarriers.emplace_back(bmb); }
        void add(ref_ptr<ImageMemoryBarrier> imb) { imageMemoryBarriers.emplace_back(imb); }

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
