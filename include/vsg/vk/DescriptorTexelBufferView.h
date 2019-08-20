#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferView.h>
#include <vsg/vk/Descriptor.h>

namespace vsg
{
    using BufferViewList = std::vector<ref_ptr<BufferView>>;

    class VSG_DECLSPEC DescriptorTexelBufferView : public Inherit<Descriptor, DescriptorTexelBufferView>
    {
    public:
        DescriptorTexelBufferView(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const BufferViewList& texelBufferViews);

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        uint32_t getNumDescriptors() const override { return static_cast<uint32_t>(_texelBufferViewList.size()); }

    protected:
        BufferViewList _texelBufferViewList;
        std::vector<VkBufferView> _texelBufferViews;
    };
    VSG_type_name(vsg::DescriptorTexelBufferView)

    struct Material
    {
        vec4 ambientColor;
        vec4 diffuseColor;
        vec4 specularColor;
        float shine;
    };

    class VSG_DECLSPEC MaterialValue : public Inherit<Value<Material>, MaterialValue>
    {
    public:
        MaterialValue() {}

        void read(Input& input) override
        {
            value().ambientColor = input.readValue<vec4>("ambientColor");
            value().diffuseColor = input.readValue<vec4>("diffuseColor");
            value().specularColor = input.readValue<vec4>("specularColor");
            value().shine = input.readValue<float>("shine");
        }
        void write(Output& output) const override
        {
            output.writeValue<vec4>("ambientColor", value().ambientColor);
            output.writeValue<vec4>("diffuseColor", value().diffuseColor);
            output.writeValue<vec4>("specularColor", value().specularColor);
            output.writeValue<float>("shine", value().shine);
        }
    };
    VSG_type_name(vsg::MaterialValue)

} // namespace vsg
