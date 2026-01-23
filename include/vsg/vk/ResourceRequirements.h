#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Bin.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/vk/DescriptorPool.h>

#include <map>
#include <set>
#include <stack>

namespace vsg
{
    /// ResourceRequirements provides a container for various Vulkan resource requirements that can be used to help guide allocation of resources.
    class VSG_DECLSPEC ResourceRequirements
    {
    public:
        ResourceRequirements();
        ResourceRequirements(const ResourceRequirements& rhs) = default;
        explicit ResourceRequirements(ref_ptr<ResourceHints> hints);

        ResourceRequirements& operator=(const ResourceRequirements& rhs) = default;

        void apply(const ResourceHints& resourceHints);

        uint32_t computeNumDescriptorSets() const;
        DescriptorPoolSizes computeDescriptorPoolSizes() const;

        struct ViewDetails
        {
            std::set<int32_t> indices;
            std::set<const Bin*> bins;
            std::set<const Light*> lights;

            void add(ViewDetails& vd)
            {
                indices.insert(vd.indices.begin(), vd.indices.end());
                bins.insert(vd.bins.begin(), vd.bins.end());
                lights.insert(vd.lights.begin(), vd.lights.end());
            }
        };

        using Descriptors = std::set<const Descriptor*>;
        using DescriptorSets = std::set<const DescriptorSet*>;
        using DescriptorTypeMap = std::map<VkDescriptorType, uint32_t>;
        using Views = std::map<const View*, ViewDetails>;
        using ViewDetailStack = std::stack<ViewDetails>;

        DynamicData dynamicData;

        Descriptors descriptors;
        DescriptorSets descriptorSets;
        DescriptorTypeMap descriptorTypeMap;
        Views views;
        ViewDetailStack viewDetailsStack;

        Slots maxSlots;
        uint32_t externalNumDescriptorSets = 0;
        bool containsPagedLOD = false;

        struct BufferProperties
        {
            VkBufferUsageFlags usageFlags = 0;
            VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            bool operator < (const BufferProperties& rhs) const { return (usageFlags < rhs.usageFlags) || ((usageFlags == rhs.usageFlags) && (sharingMode < rhs.sharingMode)); }
        };

        std::map<BufferProperties, std::set<ref_ptr<BufferInfo>>> bufferInfos;
        std::set<ref_ptr<ImageInfo>> imageInfos;

        VkDeviceSize bufferMemoryRequirements = 0;
        VkDeviceSize imageMemoryRequirements = 0;

        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumDeviceMemorySize = 16 * 1024 * 1024;

        VkDeviceSize minimumStagingBufferSize = 16 * 1024 * 1024;

        uivec2 numLightsRange = {8, 1024};
        uivec2 numShadowMapsRange = {0, 64};
        uivec2 shadowMapSize = {2048, 2048};

        DataTransferHint dataTransferHint = COMPILE_TRAVERSAL_USE_TRANSFER_TASK;
        uint32_t viewportStateHint = DYNAMIC_VIEWPORTSTATE;
    };
    VSG_type_name(vsg::ResourceRequirements);

    /// CollectResourceRequirements is a visitor class that collects the ResourceRequirements of a scene graph
    class VSG_DECLSPEC CollectResourceRequirements : public Inherit<ConstVisitor, CollectResourceRequirements>
    {
    public:
        CollectResourceRequirements() { overrideMask = vsg::MASK_ALL; }

        ResourceRequirements requirements;

        /// create ResouceHints that capture the collected ResourceRequirements. Note, call after the CollectResourceRequirements traversal.
        ref_ptr<ResourceHints> createResourceHints(uint32_t tileMultiplier = 1) const;

        using ConstVisitor::apply;

        bool checkForResourceHints(const Object& object);

        void apply(const Object& object) override;
        void apply(const ResourceHints& resourceHints) override;
        void apply(const Node& node) override;
        void apply(const StateCommand& stateCommand) override;
        void apply(const DescriptorSet& descriptorSet) override;
        void apply(const Descriptor& descriptor) override;
        void apply(const DescriptorBuffer& descriptorBuffer) override;
        void apply(const DescriptorImage& descriptorImage) override;
        void apply(const PagedLOD& plod) override;
        void apply(const Light& light) override;
        void apply(const RenderGraph& rg) override;
        void apply(const View& view) override;
        void apply(const DepthSorted& depthSorted) override;
        void apply(const Layer& layer) override;
        void apply(const Bin& bin) override;
        void apply(const Geometry& geometry) override;
        void apply(const VertexDraw& vid) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const BindIndexBuffer& bib) override;
        void apply(const InstanceNode& in) override;
        void apply(const InstanceDraw& id) override;
        void apply(const InstanceDrawIndexed& idi) override;

        using BufferProperties = ResourceRequirements::BufferProperties;

        virtual void apply(ref_ptr<BufferInfo> bufferInfo, BufferProperties bufferProperties);
        virtual void apply(ref_ptr<ImageInfo> imageInfo);

    protected:
        bool registerDescriptor(const Descriptor& descriptor);
    };
    VSG_type_name(vsg::CollectResourceRequirements);

} // namespace vsg
