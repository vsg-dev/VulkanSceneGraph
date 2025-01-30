#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/ArrayState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/utils/CoordinateSpace.h>

namespace vsg
{

    struct VSG_DECLSPEC AttributeBinding
    {
        std::string name;
        std::string define;
        uint32_t location = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
        CoordinateSpace coordinateSpace = CoordinateSpace::NO_PREFERENCE;
        ref_ptr<Data> data;

        int compare(const AttributeBinding& rhs) const;

        explicit operator bool() const noexcept { return !name.empty(); }
    };
    VSG_type_name(vsg::AttributeBinding);

    struct VSG_DECLSPEC DescriptorBinding
    {
        std::string name;
        std::string define;
        uint32_t set = 0;
        uint32_t binding = 0;
        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uint32_t descriptorCount = 0;
        VkShaderStageFlags stageFlags = 0;
        CoordinateSpace coordinateSpace = CoordinateSpace::NO_PREFERENCE;
        ref_ptr<Data> data;

        int compare(const DescriptorBinding& rhs) const;

        explicit operator bool() const noexcept { return !name.empty(); }
    };
    VSG_type_name(vsg::DescriptorBinding);

    struct VSG_DECLSPEC PushConstantRange
    {
        std::string name;
        std::string define;
        VkPushConstantRange range;

        int compare(const PushConstantRange& rhs) const;
    };
    VSG_type_name(vsg::PushConstantRange);

    struct VSG_DECLSPEC DefinesArrayState
    {
        std::set<std::string> defines;
        ref_ptr<ArrayState> arrayState;

        int compare(const DefinesArrayState& rhs) const;
    };
    VSG_type_name(vsg::DefinesArrayState);

    /// Base class for specifying custom DescriptorSetLayout and StateCommand
    struct VSG_DECLSPEC CustomDescriptorSetBinding : public Inherit<Object, CustomDescriptorSetBinding>
    {
        explicit CustomDescriptorSetBinding(uint32_t in_set = 0);

        uint32_t set = 0;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        virtual bool compatibleDescriptorSetLayout(const DescriptorSetLayout& dsl) const = 0;
        virtual ref_ptr<DescriptorSetLayout> createDescriptorSetLayout() = 0;
        virtual ref_ptr<StateCommand> createStateCommand(ref_ptr<PipelineLayout> layout) = 0;
    };
    VSG_type_name(vsg::CustomDescriptorSetBinding);

    /// Custom state binding class for providing the DescriptorSetLayout and StateCommand required to pass view dependent data, lights/shadows etc., to shaders
    struct VSG_DECLSPEC ViewDependentStateBinding : public Inherit<CustomDescriptorSetBinding, ViewDependentStateBinding>
    {
        explicit ViewDependentStateBinding(uint32_t in_set = 0);

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        ref_ptr<DescriptorSetLayout> viewDescriptorSetLayout;

        bool compatibleDescriptorSetLayout(const DescriptorSetLayout& dsl) const override;
        ref_ptr<DescriptorSetLayout> createDescriptorSetLayout() override;
        ref_ptr<StateCommand> createStateCommand(ref_ptr<PipelineLayout> layout) override;
    };
    VSG_type_name(vsg::ViewDependentStateBinding);

    /// ShaderSet provides a collection of shader related settings to provide a form of shader introspection.
    class VSG_DECLSPEC ShaderSet : public Inherit<Object, ShaderSet>
    {
    public:
        ShaderSet();
        explicit ShaderSet(const ShaderStages& in_stages, ref_ptr<ShaderCompileSettings> in_hints = {});

        /// base ShaderStages that other variants are based on.
        ShaderStages stages;

        std::vector<AttributeBinding> attributeBindings;
        std::vector<DescriptorBinding> descriptorBindings;
        std::vector<PushConstantRange> pushConstantRanges;
        std::vector<DefinesArrayState> definesArrayStates; // put more constrained ArrayState matches first so they are matched first.
        std::set<std::string> optionalDefines;
        GraphicsPipelineStates defaultGraphicsPipelineStates;
        std::vector<ref_ptr<CustomDescriptorSetBinding>> customDescriptorSetBindings;

        ref_ptr<ShaderCompileSettings> defaultShaderHints;
        /// variants of the rootShaderModule compiled for different combinations of ShaderCompileSettings
        std::map<ref_ptr<ShaderCompileSettings>, ShaderStages, DereferenceLess> variants;

        /// mutex used by getShaderStages(..) to ensure the variants map can be used from multiple threads.
        std::mutex mutex;

        /// add an attribute binding, Not thread safe, should only be called when initially setting up the ShaderSet
        void addAttributeBinding(const std::string& name, const std::string& define, uint32_t location, VkFormat format, ref_ptr<Data> data, CoordinateSpace coordinateSpace = CoordinateSpace::NO_PREFERENCE);

        /// add an uniform binding. Not thread safe, should only be called when initially setting up the ShaderSet
        void addDescriptorBinding(const std::string& name, const std::string& define, uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Data> data, CoordinateSpace coordinateSpace = CoordinateSpace::NO_PREFERENCE);

        [[deprecated("use addDescriptorBinding(..)")]] void addUniformBinding(const std::string& name, const std::string& define, uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Data> data) { addDescriptorBinding(name, define, set, binding, descriptorType, descriptorCount, stageFlags, data); }

        /// add a push constant range. Not thread safe, should only be called when initially setting up the ShaderSet
        void addPushConstantRange(const std::string& name, const std::string& define, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);

        /// get the AttributeBinding associated with name
        AttributeBinding& getAttributeBinding(const std::string& name);

        /// get the const AttributeBinding associated with name
        const AttributeBinding& getAttributeBinding(const std::string& name) const;

        /// get the DescriptorBinding associated with name
        DescriptorBinding& getDescriptorBinding(const std::string& name);

        /// get the const DescriptorBinding associated with name
        const DescriptorBinding& getDescriptorBinding(const std::string& name) const;

        [[deprecated("use getDescriptorBinding(..)")]] DescriptorBinding& getUniformBinding(const std::string& name) { return getDescriptorBinding(name); }

        [[deprecated("use getDescriptorBinding(..)")]] const DescriptorBinding& getUnifomrBinding(const std::string& name) const { return getDescriptorBinding(name); }

        /// get the first ArrayState that has matches with defines in the specified list of defines.
        ref_ptr<ArrayState> getSuitableArrayState(const std::set<std::string>& defines) const;

        /// get the ShaderStages variant that uses specified ShaderCompileSettings.
        ShaderStages getShaderStages(ref_ptr<ShaderCompileSettings> scs = {});

        /// return the <minimum_set, maximum_set+1> range of set numbers encompassing DescriptorBindings
        std::pair<uint32_t, uint32_t> descriptorSetRange() const;

        /// return true of specified descriptor set layout is compatible with what is required for this ShaderSet
        virtual bool compatibleDescriptorSetLayout(const DescriptorSetLayout& dsl, const std::set<std::string>& defines, uint32_t set) const;

        /// create the descriptor set layout.
        virtual ref_ptr<DescriptorSetLayout> createDescriptorSetLayout(const std::set<std::string>& defines, uint32_t set) const;

        /// return true of specified pipline layout is compatible with what is required for this ShaderSet
        virtual bool compatiblePipelineLayout(const PipelineLayout& layout, const std::set<std::string>& defines) const;

        /// create the pipeline layout for all descriptor sets enabled by specified defines or required by default.
        inline ref_ptr<PipelineLayout> createPipelineLayout(const std::set<std::string>& defines) { return createPipelineLayout(defines, descriptorSetRange()); }

        /// create pipeline layout for specified range <minimum_set, maximum_set+1> of descriptor sets that are enabled by specified defines or required by default.
        ///
        /// Note: the underlying Vulkan call vkCreatePipelineLayout assumes that the array of
        /// descriptor sets starts with set 0. Therefore the minimum_set argument should be 0 unless
        /// you really know what you're doing.
        virtual ref_ptr<PipelineLayout> createPipelineLayout(const std::set<std::string>& defines, std::pair<uint32_t, uint32_t> range) const;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~ShaderSet();

        AttributeBinding _nullAttributeBinding;
        DescriptorBinding _nullDescriptorBinding;
    };
    VSG_type_name(vsg::ShaderSet);

    /// create a ShaderSet for unlit, flat shaded rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createFlatShadedShaderSet(ref_ptr<const Options> options = {});

    /// create a ShaderSet for Phong shaded rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createPhongShaderSet(ref_ptr<const Options> options = {});

    /// create a ShaderSet for Physics Based Rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createPhysicsBasedRenderingShaderSet(ref_ptr<const Options> options = {});

} // namespace vsg
