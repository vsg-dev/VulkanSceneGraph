#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/core/type_name.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{

    struct ModifiedCount
    {
        uint32_t count = 0;

        bool operator==(const ModifiedCount& rhs) const { return count == rhs.count; }
        bool operator!=(const ModifiedCount& rhs) const { return count != rhs.count; }

        void operator++() { ++count; }
    };

    /** 64 bit block of compressed texel data.*/
    using block64 = uint8_t[8];

    /** 128 bit block of compressed texel data.*/
    using block128 = uint8_t[16];

    enum Origin : uint8_t
    {
        TOP_LEFT = 0,
        BOTTOM_LEFT = 2
    };

    template<typename T>
    struct stride_iterator
    {
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        value_type* ptr;
        uint32_t stride; // stride in bytes

        inline void advance()
        {
            if constexpr (std::is_const<value_type>::value)
                ptr = reinterpret_cast<value_type*>(reinterpret_cast<const uint8_t*>(ptr) + stride);
            else
                ptr = reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(ptr) + stride);
        }

        stride_iterator& operator++()
        {
            advance();
            return *this;
        }
        stride_iterator operator++(int)
        {
            stride_iterator reval(*this);
            advance();
            return reval;
        }

        bool operator==(stride_iterator rhs) const { return ptr == rhs.ptr; }
        bool operator!=(stride_iterator rhs) const { return ptr != rhs.ptr; }
        bool operator<(stride_iterator rhs) const { return ptr < rhs.ptr; }
        bool operator<=(stride_iterator rhs) const { return ptr <= rhs.ptr; }
        bool operator>(stride_iterator rhs) const { return ptr > rhs.ptr; }
        bool operator>=(stride_iterator rhs) const { return ptr >= rhs.ptr; }

        value_type& operator*() { return *reinterpret_cast<value_type*>(ptr); }
        value_type* operator->() { return reinterpret_cast<value_type*>(ptr); }
    };

    class VSG_DECLSPEC Data : public Object
    {
    public:
        /* Layout used for specifying the format of the data, use of mipmaps, block compressed data and origin.
         * Default of no mipmapping and {1,1,1} is uncompressed.
         * A single block (Block64/Block128) is stored as a single value with the Data object. */
        struct Layout
        {
            VkFormat format = VK_FORMAT_UNDEFINED;
            uint32_t stride = 0;
            uint8_t maxNumMipmaps = 0;
            uint8_t blockWidth = 1;
            uint8_t blockHeight = 1;
            uint8_t blockDepth = 1;
            uint8_t origin = TOP_LEFT; /// Hint for setting up texture coordinates, bit 0 x/width axis, bit 1 y/height axis, bit 2 z/depth axis. Vulkan origin for images is top left, which is denoted as 0 here.
            int8_t imageViewType = -1; /// -1 signifies undefined VkImageViewType, if value >=0 then value should be treated as valid VkImageViewType
        };

        Data() {}

        explicit Data(Layout layout) :
            _layout(layout) {}

        Data(Layout layout, uint32_t min_stride) :
            _layout(layout)
        {
            if (_layout.stride < min_stride) _layout.stride = min_stride;
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(Data); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Data) == type ? true : Object::is_compatible(type); }

        void read(Input& input) override;
        void write(Output& output) const override;

        /** Set Layout */
        void setLayout(Layout layout)
        {
            VkFormat previous_format = _layout.format; // temporary hack to keep applications that call setFormat(..) before setLayout(..) working
            uint32_t previous_stride = _layout.stride;
            _layout = layout;
            if (_layout.format == 0 && previous_format != 0) _layout.format = previous_format; // temporary hack to keep existing applications working
            if (_layout.stride == 0 && previous_stride != 0) _layout.stride = previous_stride; // make sure the layout as a valid stride.
        }

        /** Get the Layout.*/
        Layout& getLayout() { return _layout; }

        /** Get the Layout.*/
        Layout getLayout() const { return _layout; }

        virtual std::size_t valueSize() const = 0;
        virtual std::size_t valueCount() const = 0;

        virtual std::size_t dataSize() const = 0;

        virtual void* dataPointer() = 0;
        virtual const void* dataPointer() const = 0;

        virtual void* dataPointer(size_t index) = 0;
        virtual const void* dataPointer(size_t index) const = 0;

        virtual void* dataRelease() = 0;

        virtual std::uint32_t dimensions() const = 0;

        virtual std::uint32_t width() const = 0;
        virtual std::uint32_t height() const = 0;
        virtual std::uint32_t depth() const = 0;

        bool contigous() const { return valueSize() == _layout.stride; }

        uint32_t stride() const { return _layout.stride ? _layout.stride : static_cast<uint32_t>(valueSize()); }

        using MipmapOffsets = std::vector<std::size_t>;
        MipmapOffsets computeMipmapOffsets() const;
        static std::size_t computeValueCountIncludingMipmaps(std::size_t w, std::size_t h, std::size_t d, uint32_t maxNumMipmaps);

        /// increment the ModifiedCount to signify the data has been modified
        void dirty() { ++_modifiedCount; }

        /// get the Data's ModifiedCount and return true if this changes the specified ModifiedCount
        bool getModifiedCount(ModifiedCount& mc) const
        {
            if (_modifiedCount != mc)
            {
                mc = _modifiedCount;
                return true;
            }
            else
                return false;
        }

        /// return true if Data's ModifiedCount is diffferent than the specified ModifiedCount
        bool differentModifiedCount(const ModifiedCount& mc) const { return _modifiedCount != mc; }

    protected:
        virtual ~Data() {}

        Layout _layout;
        ModifiedCount _modifiedCount;
    };
    VSG_type_name(vsg::Data);

    using DataList = std::vector<ref_ptr<Data>>;

} // namespace vsg
