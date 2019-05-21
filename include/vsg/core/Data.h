#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{

    /** 64 bit block of compressed texel data.*/
    using block64 = uint8_t[8];

    /** 128 bit block of compressed texel data.*/
    using block128 = uint8_t[16];

    class VSG_DECLSPEC Data : public Object
    {
    public:
        /* Layout used for configuruing use of mipmaps and block compressed data.
         * Default of no mipmapping and {1,1,1} is uncompessed.
         * A single block (Block64/Block128) is stored as a single value with the Data object. */
        struct Layout
        {
            uint8_t maxNumMipmaps = 0;
            uint8_t blockWidth = 1;
            uint8_t blockHeight = 1;
            uint8_t blockDepth = 1;
        };

        Data() {}

        explicit Data(VkFormat format) :
            _format(format) {}

        explicit Data(Layout layout) :
            _layout(layout) {}

        Data(VkFormat format, Layout layout) :
            _format(format),
            _layout(layout) {}

        std::size_t sizeofObject() const noexcept override { return sizeof(Data); }

        void read(Input& input) override;
        void write(Output& output) const override;

        void setFormat(VkFormat format) { _format = format; }
        VkFormat getFormat() const { return _format; }

        /** Set Layout */
        void setLayout(Layout layout) { _layout = layout; }

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

        virtual std::uint32_t width() const = 0;
        virtual std::uint32_t height() const = 0;
        virtual std::uint32_t depth() const = 0;

        using MipmapOffsets = std::vector<std::size_t>;
        MipmapOffsets computeMipmapOffsets() const;
        static std::size_t computeValueCountIncludingMipmaps(std::size_t w, std::size_t h, std::size_t d, uint32_t maxNumMipmaps);

    protected:
        virtual ~Data() {}

        VkFormat _format = VK_FORMAT_UNDEFINED;
        Layout _layout;
    };
} // namespace vsg
