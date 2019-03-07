#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/Descriptor.h>

#define VSG_uniformValue(N, T) \
    using N = BasicUniformValue<T>; \
    template<>          \
    constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

namespace vsg
{
    template<typename T>
    class UniformValue : public Data
    {
    public:
        using value_type = T;

        UniformValue() {}
        UniformValue(const UniformValue& rhs) :
            _value(rhs._value) {}
        explicit UniformValue(const value_type& in_value) :
            _value(in_value) {}

        std::size_t sizeofObject() const noexcept override { return sizeof(UniformValue); }

        void accept(Visitor& visitor) override { visitor.apply(*this); }
        void accept(ConstVisitor& visitor) const override { visitor.apply(*this); }

        const char* className() const noexcept override { return type_name<UniformValue>(); }

        void read(Input& input) override
        {
            Data::read(input);
        }

        void write(Output& output) const override
        {
            Data::write(output);
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return 1; }

        std::size_t dataSize() const override { return sizeof(value_type); }
        void* dataPointer() override { return &_value; }
        const void* dataPointer() const override { return &_value; }
        void* dataRelease() override { return nullptr; }

        std::size_t width() const override { return 1; }
        std::size_t height() const override { return 1; }
        std::size_t depth() const override { return 1; }

        UniformValue& operator=(const UniformValue& rhs)
        {
            _value = rhs._value;
            return *this;
        }
        UniformValue& operator=(const value_type& rhs)
        {
            _value = rhs;
            return *this;
        }

        operator value_type&() { return _value; }
        operator const value_type&() const { return _value; }

        value_type& value() { return _value; }
        const value_type& value() const { return _value; }

    protected:
        virtual ~UniformValue() {}

    private:
        value_type _value;
    };

    template<typename T>
    class BasicUniformValue : public UniformValue<T>
    {
        using base_type = UniformValue<T>;

        void read(Input& input) override
        {
            base_type::read(input);
            input.read("Value", value());
        }

        void write(Output& output) const override
        {
            base_type::write(output);
            output.write("Value", value());
        }
    };

    VSG_uniformValue(boolUniformValue, bool);
    VSG_uniformValue(intUniformValue, int);
    VSG_uniformValue(uinUniformValue, unsigned int);
    VSG_uniformValue(floatUniformValue, float);
    VSG_uniformValue(doubleUniformValue, double);

    VSG_uniformValue(vec2UniformValue, vec2);
    VSG_uniformValue(vec3UniformValue, vec3);
    VSG_uniformValue(vec4UniformValue, vec4);

    VSG_uniformValue(dvec2UniformValue, dvec2);
    VSG_uniformValue(dvec3UniformValue, dvec3);
    VSG_uniformValue(dvec4UniformValue, dvec4);

    VSG_uniformValue(ubvec2UniformValue, ubvec2);
    VSG_uniformValue(ubvec3UniformValue, ubvec3);
    VSG_uniformValue(ubvec4UniformValue, ubvec4);

    VSG_uniformValue(mat3UniformValue, mat3);
    VSG_uniformValue(dmat3UniformValue, dmat3);

    VSG_uniformValue(mat4UniformValue, mat4);
    VSG_uniformValue(dmat4UniformValue, dmat4);

    class VSG_DECLSPEC Uniform : public Inherit<Descriptor, Uniform>
    {
    public:
        Uniform();

        void compile(Context& context) override;

        void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        // settings
        DataList _dataList;

        ref_ptr<vsg::Descriptor> _implementation;
    };
    VSG_type_name(vsg::Uniform)

} // namespace vsg
