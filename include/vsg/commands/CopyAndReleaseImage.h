#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/ImageInfo.h>
#include <vsg/vk/MemoryBufferPools.h>

#include <vsg/commands/Command.h>

namespace vsg
{

    class VSG_DECLSPEC CopyAndReleaseImage : public Inherit<Command, CopyAndReleaseImage>
    {
    public:
        CopyAndReleaseImage(ref_ptr<MemoryBufferPools> optional_stagingMemoryBufferPools = {});

        struct VSG_DECLSPEC CopyData
        {
            CopyData() {}
            CopyData(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels = 1);

            ref_ptr<BufferInfo> source;
            ref_ptr<ImageInfo> destination;

            uint32_t mipLevels = 1;

            Data::Layout layout;
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;
            Data::MipmapOffsets mipmapOffsets;

            void record(CommandBuffer& commandBuffer) const;
        };

        void add(const CopyData& cd);
        void add(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest);
        void add(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels);

        /// MemoryBufferPools used for allocation staging buffer used by the copy(ref_ptr<Data>, ImageInfo) method.  Users should assign a MemoryBufferPools with appropriate settings.
        ref_ptr<MemoryBufferPools> stagingMemoryBufferPools;

        /// copy data into a staging buffer and then use copy command to transfer this to the GPU image specified by ImageInfo
        void copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest);
        void copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels);

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~CopyAndReleaseImage();

        void _copyDirectly(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels);

        mutable std::mutex _mutex;
        mutable std::vector<CopyData> _pending;
        mutable std::vector<CopyData> _completed;
        mutable std::vector<CopyData> _readyToClear;
    };

} // namespace vsg
