#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Object.h>
#include <vsg/core/compare.h>
#include <vsg/core/type_name.h>
#include <vsg/vk/vulkan.h>

#include <cstring>
#include <vector>

namespace vsg
{

    /// ModifiedCount provides a count value to keep track of modifications to data.
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

    enum DataVariance : uint8_t
    {
        STATIC_DATA = 0,                       /** treat data as if doesn't not change .*/
        STATIC_DATA_UNREF_AFTER_TRANSFER = 1,  /** unref this vsg::Data after the data has been transferred to the GPU memory .*/
        DYNAMIC_DATA = 2,                      /** data is updated prior to the record traversal and will need transferring to GPU memory.*/
        DYNAMIC_DATA_TRANSFER_AFTER_RECORD = 3 /** data is updated during the record traversal and will need transferring to GPU memory.*/
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

    /// Data base class for abstracting data such a values, vertices, images etc.
    /// Main subclasses are vsg::Value, vsg::Array, vsg::Array2D and vsg::Array3D.
    class VSG_DECLSPEC Data : public Object
    {
    public:
        /* Properties used for specifying the format of the data, use of mipmaps, block compressed data and origin.
         * Default of no mipmapping and {1,1,1} is uncompressed.
         * A single block (Block64/Block128) is stored as a single value with the Data object. */
        struct VSG_DECLSPEC Properties
        {
            Properties() = default;
            Properties(const Properties& rhs) = default;
            explicit Properties(VkFormat in_format) :
                format(in_format) {}

            VkFormat format = VK_FORMAT_UNDEFINED;
            uint32_t stride = 0;
            uint8_t maxNumMipmaps = 0;
            uint8_t blockWidth = 1;
            uint8_t blockHeight = 1;
            uint8_t blockDepth = 1;
            uint8_t origin = TOP_LEFT;               /// Hint for setting up texture coordinates, bit 0 x/width axis, bit 1 y/height axis, bit 2 z/depth axis. Vulkan origin for images is top left, which is denoted as 0 here.
            int8_t imageViewType = -1;               /// -1 signifies undefined VkImageViewType, if value >=0 then value should be treated as valid VkImageViewType.
            DataVariance dataVariance = STATIC_DATA; /// hint as how the data values may change during the lifetime of the vsg::Data.
            AllocatorType allocatorType = ALLOCATOR_TYPE_VSG_ALLOCATOR;

            int compare(const Properties& rhs) const;
            Properties& operator=(const Properties& rhs);
        };

        Data() {}

        explicit Data(Properties layout) :
            properties(layout) {}

        Data(Properties layout, uint32_t min_stride) :
            properties(layout)
        {
            if (properties.stride < min_stride) properties.stride = min_stride;
        }

        /// provide new and delete to enable custom memory management via the vsg::Allocator singleton, using the MEMORY_AFFINTY_DATA
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr);

        std::size_t sizeofObject() const noexcept override { return sizeof(Data); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Data) == type || Object::is_compatible(type); }

        int compare(const Object& rhs_object) const override
        {
            int result = Object::compare(rhs_object);
            if (result != 0) return result;

            auto& rhs = static_cast<decltype(*this)>(rhs_object);

            if ((result = properties.compare(rhs.properties))) return result;

            // the shorter data is less
            if (dataSize() < rhs.dataSize()) return -1;
            if (dataSize() > rhs.dataSize()) return 1;

            // if both empty then they must be equal
            if (dataSize() == 0) return 0;

            // use memcpy to compare the contents of the data
            return std::memcmp(dataPointer(), rhs.dataPointer(), dataSize());
        }

        void read(Input& input) override;
        void write(Output& output) const override;

        /// properties of the data such as format, origin, stride, dataVariance etc.
        Properties properties;

        bool dynamic() const { return properties.dataVariance >= DYNAMIC_DATA; }

        virtual std::size_t valueSize() const = 0;
        virtual std::size_t valueCount() const = 0;

        virtual bool dataAvailable() const = 0;
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

        bool contiguous() const { return valueSize() == properties.stride; }

        uint32_t stride() const { return properties.stride ? properties.stride : static_cast<uint32_t>(valueSize()); }

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

        /// return true if Data's ModifiedCount is different than the specified ModifiedCount
        bool differentModifiedCount(const ModifiedCount& mc) const { return _modifiedCount != mc; }

    protected:
        virtual ~Data() {}

        ModifiedCount _modifiedCount;

#if 1
    public:
        /// deprecated: provided for backwards compatibility, use Properties instead.
        using Layout = Properties;

        /// deprecated: use data->properties = properties instead.
        void setLayout(Layout layout)
        {
            VkFormat previous_format = properties.format; // temporary hack to keep applications that call setFormat(..) before setProperties(..) working
            uint32_t previous_stride = properties.stride;
            properties = layout;
            if (properties.format == 0 && previous_format != 0) properties.format = previous_format; // temporary hack to keep existing applications working
            if (properties.stride == 0 && previous_stride != 0) properties.stride = previous_stride; // make sure the layout as a valid stride.
        }
        /// deprecated: use data->properties
        Layout& getLayout() { return properties; }
        /// deprecated: use data->properties
        Layout getLayout() const { return properties; }
#endif
    };
    VSG_type_name(vsg::Data);

    using DataList = std::vector<ref_ptr<Data>>;

} // namespace vsg
