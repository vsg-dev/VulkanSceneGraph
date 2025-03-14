#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/vec2.h>
#include <vsg/vk/DescriptorPool.h>

namespace vsg
{

    enum DataTransferHint
    {
        COMPILE_TRAVERSAL_TRANSFER_DATA_AND_WAIT,
        COMPILE_TRAVERSAL_USE_TRANSFER_TASK
    };

    enum ViewportStateMask : uint32_t
    {
        STATIC_VIEWPORTSTATE = 1 << 0,
        DYNAMIC_VIEWPORTSTATE = 1 << 1
    };

    /// ResourceHints provides settings that help preallocation of Vulkan resources and memory.
    class VSG_DECLSPEC ResourceHints : public Inherit<Object, ResourceHints>
    {
    public:
        ResourceHints();

        bool empty() const noexcept { return maxStateSlot == 0 && maxViewSlot == 0 && numDescriptorSets == 0 && descriptorPoolSizes.empty(); }

        uint32_t maxStateSlot = 0;
        uint32_t maxViewSlot = 0;
        uint32_t numDescriptorSets = 0;
        DescriptorPoolSizes descriptorPoolSizes;

        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumDeviceMemorySize = 16 * 1024 * 1024;

        VkDeviceSize minimumStagingBufferSize = 16 * 1024 * 1024;

        uivec2 numLightsRange = {8, 1024};
        uivec2 numShadowMapsRange = {0, 64};
        uivec2 shadowMapSize = {2048, 2048};

        uint32_t numDatabasePagerReadThreads = 4;

        DataTransferHint dataTransferHint = COMPILE_TRAVERSAL_USE_TRANSFER_TASK;
        uint32_t viewportStateHint = DYNAMIC_VIEWPORTSTATE;

    public:
        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~ResourceHints();
    };
    VSG_type_name(vsg::ResourceHints);

} // namespace vsg
