#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/vulkan.h>

namespace vsg
{

    /** template helper class that decomposed draw(..) and drawIndexed(..) calls into individual points, lines or triangles.*/
    template<class T>
    struct PrimitiveFunctor : public T
    {
        template<typename... Args>
        PrimitiveFunctor(Args&&... args) :
            T(std::forward<Args>(args)...) {}

        void draw(VkPrimitiveTopology topology, uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
        {
            uint32_t lastIndex = instanceCount > 1 ? (firstInstance + instanceCount) : firstInstance + 1;
            for (uint32_t instanceIndex = firstInstance; instanceIndex < lastIndex; ++instanceIndex)
            {
                T::instance(instanceIndex);

                switch(topology)
                {
                    case(VK_PRIMITIVE_TOPOLOGY_POINT_LIST):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP):
                        break;
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST):
                    {
                        uint32_t primtiveCount = vertexCount / 3;
                        uint32_t endVertex = firstVertex + primtiveCount * 3;
                        for (uint32_t i = firstVertex; i < endVertex; i += 3)
                        {
                            T::triangle(i, i + 1, i + 2);
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST):
                    default:
                        break;
                }
            }
        }

        template<class IndexArray>
        void drawIndexed(VkPrimitiveTopology topology, IndexArray indices, uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount)
        {
            uint32_t lastIndex = instanceCount > 1 ? (firstInstance + instanceCount) : firstInstance + 1;
            for (uint32_t instanceIndex = firstInstance; instanceIndex < lastIndex; ++instanceIndex)
            {
                T::instance(instanceIndex);

                switch(topology)
                {
                    case(VK_PRIMITIVE_TOPOLOGY_POINT_LIST):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP):
                        break;
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST):
                    {
                        uint32_t primtiveCount = indexCount / 3;
                        uint32_t endIndex = firstIndex + primtiveCount * 3;
                        for (uint32_t i = firstIndex; i < endIndex; i += 3)
                        {
                            T::triangle(indices->at(i), indices->at(i + 1), indices->at(i + 2));
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST):
                    default:
                        break;
                }
            }
        }
    };


} // namespace vsg
