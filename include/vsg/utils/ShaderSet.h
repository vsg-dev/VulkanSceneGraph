#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/traversals/ArrayState.h>

namespace vsg
{

    struct AttributeBinding
    {
        std::string name;
        std::string define;
        uint32_t location = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
        ref_ptr<Data> data;

        int compare(const AttributeBinding& rhs) const;

        explicit operator bool() const noexcept { return !name.empty(); }
    };

    struct UniformBinding
    {
        std::string name;
        std::string define;
        uint32_t set = 0;
        uint32_t binding = 0;
        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uint32_t descriptorCount = 0;
        VkShaderStageFlags stageFlags = 0;
        ref_ptr<Data> data;

        int compare(const UniformBinding& rhs) const;

        explicit operator bool() const noexcept { return !name.empty(); }
    };

    struct PushConstantRange
    {
        std::string name;
        std::string define;
        VkPushConstantRange range;

        int compare(const PushConstantRange& rhs) const;
    };

    struct DefinesArrayState
    {
        std::vector<std::string> defines;
        ref_ptr<ArrayState> arrayState;
    };

    class VSG_DECLSPEC ShaderSet : public Inherit<Object, ShaderSet>
    {
    public:
        ShaderSet();
        ShaderSet(const ShaderStages& in_stages);

        /// base ShaderStages that other variants as based on.
        ShaderStages stages;

        std::vector<AttributeBinding> attributeBindings;
        std::vector<UniformBinding> uniformBindings;
        std::vector<PushConstantRange> pushConstantRanges;
        std::vector<DefinesArrayState> definesArrayStates; // put more constrained ArrayState matches first so they are matched first.

        /// variants of the rootShaderModule compiled for differen combinations of ShaderCompileSettings
        std::map<ref_ptr<ShaderCompileSettings>, ShaderStages, DerefenceLess> variants;

        /// mutex used be getShaderStages(..) so ensure the variants map can be used from multiple threads.
        std::mutex mutex;

        /// add an attribute binding, Not thread safe, should only be called when initially setting up the ShaderSet
        void addAttributeBinding(std::string name, std::string define, uint32_t location, VkFormat format, ref_ptr<Data> data);

        /// add an uniform binding. Not thread safe, should only be called when initially setting up the ShaderSet
        void addUniformBinding(std::string name, std::string define, uint32_t set, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, ref_ptr<Data> data);

        /// add an uniform binding. Not thread safe, should only be called when initially setting up the ShaderSet
        void addPushConstantRange(std::string name, std::string define, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);

        /// get the AttributeBinding associated with name
        const AttributeBinding& getAttributeBinding(const std::string& name) const;

        /// get the AttributeBinding associated with name
        const UniformBinding& getUniformBinding(const std::string& name) const;

        /// get the first ArrayState that has matches with defines in the specified list of defines.
        ref_ptr<ArrayState> getSuitableArrayState(const std::vector<std::string>& defines) const;

        /// get the ShaderStages varient that uses specified ShaderCompileSettings.
        ShaderStages getShaderStages(ref_ptr<ShaderCompileSettings> scs = {});

        int compare(const Object& rhs) const;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~ShaderSet();

        const AttributeBinding _nullAttributeBinding;
        const UniformBinding _nullUniformBinding;
    };
    VSG_type_name(vsg::ShaderSet);

    class VSG_DECLSPEC PositionAndDisplacementMapArrayState : public Inherit<ArrayState, PositionAndDisplacementMapArrayState>
    {
    public:
    };
    VSG_type_name(vsg::PositionAndDisplacementMapArrayState);

    class VSG_DECLSPEC DisplacementMapArrayState : public Inherit<ArrayState, DisplacementMapArrayState>
    {
    public:
    };
    VSG_type_name(vsg::DisplacementMapArrayState);

    class VSG_DECLSPEC PositionArrayState : public Inherit<ArrayState, PositionArrayState>
    {
    public:
        PositionArrayState();
        PositionArrayState(const PositionArrayState& rhs);
        PositionArrayState(const ArrayState& rhs);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        uint32_t position_attribute_location = 4;
        AttributeDetails positionAttribute;

        void apply(const VertexInputState& vas) override;
        ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override;
    };
    VSG_type_name(vsg::PositionArrayState);

    /// create a ShaderSet for unlit, flat shaded rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createFlatShadedShaderSet(ref_ptr<const Options> options = {});

    /// create a ShaderSet for Phong shaded rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createPhongShaderSet(ref_ptr<const Options> options = {});

    /// create a ShaderSet for Physics Based Rendering rendering
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createPhysicsBasedRenderingShaderSet(ref_ptr<const Options> options = {});

} // namespace vsg
