#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Value.h>
#include <vsg/maths/mat4.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class VSG_DECLSPEC AccelerationGeometry : public Inherit<Object, AccelerationGeometry>
    {
    public:
        AccelerationGeometry(Allocator* allocator = nullptr);

        void compile(Context& context);

        operator VkGeometryNV() const { return _geometry; }

        ref_ptr<Data> _verts;
        ref_ptr<Data> _indices;

        // compiled data
        BufferData _vertexBuffer;
        BufferData _indexBuffer;
        VkGeometryNV _geometry;
    };
    using AccelerationGeometries = std::vector<ref_ptr<AccelerationGeometry>>;
} // namespace vsg
