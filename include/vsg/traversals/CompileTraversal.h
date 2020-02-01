#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <deque>
#include <memory>

#include <vsg/core/Object.h>

#include <vsg/nodes/Group.h>

#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/ResourceHints.h>

#include <set>

namespace vsg
{
    class CollectDescriptorStats : public ConstVisitor
    {
    public:
        using Descriptors = std::set<const Descriptor*>;
        using DescriptorSets = std::set<const DescriptorSet*>;
        using DescriptorTypeMap = std::map<VkDescriptorType, uint32_t>;

        using ConstVisitor::apply;

        bool checkForResourceHints(const Object& object);

        void apply(const Object& object) override;
        void apply(const ResourceHints& resourceHints) override;
        void apply(const Node& node) override;
        void apply(const StateGroup& stategroup) override;
        void apply(const StateCommand& stateCommand) override;
        void apply(const DescriptorSet& descriptorSet) override;
        void apply(const Descriptor& descriptor) override;

        uint32_t computeNumDescriptorSets() const;

        DescriptorPoolSizes computeDescriptorPoolSizes() const;

        Descriptors descriptors;
        DescriptorSets descriptorSets;
        DescriptorTypeMap descriptorTypeMap;
        uint32_t maxSlot = 0;
        uint32_t externalNumDescriptorSets = 0;
    };

    class VSG_DECLSPEC CompileTraversal : public Visitor
    {
    public:
        explicit CompileTraversal(Device* in_device, BufferPreferences bufferPreferences = {});
        CompileTraversal(const CompileTraversal& ct);
        ~CompileTraversal();

        void apply(Object& object) override;
        void apply(Command& command) override;
        void apply(Commands& commands) override;
        void apply(StateGroup& stateGroup) override;
        void apply(Geometry& geometry) override;
        void apply(CommandGraph& commandGraph) override;
        void apply(RenderGraph& renderGraph) override;

        void compile(Object* object);

        ref_ptr<Fence> fence;
        ref_ptr<Semaphore> semaphore;

        Context context;
    };
} // namespace vsg
