#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Descriptor.h>

namespace vsg
{

    /** Compute the VkIndexType from Data source's value size.*/
    inline VkIndexType computeIndexType(const Data* indices)
    {
        if (indices)
        {
            switch (indices->valueSize())
            {
            case (1): return VK_INDEX_TYPE_UINT8_EXT;
            case (2): return VK_INDEX_TYPE_UINT16;
            case (4): return VK_INDEX_TYPE_UINT32;
            default: break;
            }
        }
        // nothing valid assigned
        return VK_INDEX_TYPE_MAX_ENUM;
    }

    class VSG_DECLSPEC BindIndexBuffer : public Inherit<Command, BindIndexBuffer>
    {
    public:
        BindIndexBuffer() :
            _indexType(VK_INDEX_TYPE_MAX_ENUM) {}

        BindIndexBuffer(Data* indices);

        BindIndexBuffer(const BufferData& bufferData);

        BindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType) :
            _bufferData(buffer, offset, 0),
            _indexType(indexType) {}

        void setIndices(ref_ptr<Data> indices) { _bufferData._data = indices; }
        Data* getIndices() { return _bufferData._data; }
        const Data* getIndices() const { return _bufferData._data; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindIndexBuffer();

        BufferData _bufferData;
        VkIndexType _indexType;
    };
    VSG_type_name(vsg::BindIndexBuffer);

} // namespace vsg
