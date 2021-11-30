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
#include <vsg/vk/DescriptorPool.h>

#include <map>
#include <set>
#include <stack>

namespace vsg
{
    class VSG_DECLSPEC ResourceRequirements
    {
    public:
        ResourceRequirements();

        uint32_t computeNumDescriptorSets() const;
        DescriptorPoolSizes computeDescriptorPoolSizes() const;

        struct BinDetails
        {
            std::set<int32_t> indices;
            std::set<const Bin*> bins;
        };

        using Descriptors = std::set<const Descriptor*>;
        using DescriptorSets = std::set<const DescriptorSet*>;
        using DescriptorTypeMap = std::map<VkDescriptorType, uint32_t>;
        using Views = std::map<const View*, BinDetails>;
        using BinStack = std::stack<BinDetails>;

        Descriptors descriptors;
        DescriptorSets descriptorSets;
        DescriptorTypeMap descriptorTypeMap;
        Views views;
        BinStack binStack;

        uint32_t maxSlot = 0;
        uint32_t externalNumDescriptorSets = 0;
        bool containsPagedLOD = false;

        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumBufferDeviceMemorySize = 16 * 1024 * 1024;
        VkDeviceSize minimumImageDeviceMemorySize = 16 * 1024 * 1024;
    };
    VSG_type_name(vsg::ResourceRequirements);

    class VSG_DECLSPEC CollectResourceRequirements : public Inherit<ConstVisitor, CollectResourceRequirements>
    {
    public:
        CollectResourceRequirements() {}

        ResourceRequirements requirements;

        /// create ResouceHints that capture the collected ResourceRequirements. Note, call after the CollectResourceRequirements traversal.
        ref_ptr<ResourceHints> createResourceHints(uint32_t tileMultiplier = 1) const;

        using ConstVisitor::apply;

        bool checkForResourceHints(const Object& object);

        void apply(const Object& object) override;
        void apply(const ResourceHints& resourceHints) override;
        void apply(const Node& node) override;
        void apply(const StateGroup& stategroup) override;
        void apply(const StateCommand& stateCommand) override;
        void apply(const DescriptorSet& descriptorSet) override;
        void apply(const Descriptor& descriptor) override;
        void apply(const PagedLOD& plod) override;
        void apply(const View& view) override;
        void apply(const DepthSorted& depthSorted) override;
        void apply(const Bin& bin) override;

    protected:
        uint32_t _numResourceHintsAbove = 0;
    };
    VSG_type_name(vsg::CollectResourceRequirements);

} // namespace vsg
