#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/vulkan.h>
#include <vsg/io/Logger.h>

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
                if (!T::instance(instanceIndex)) continue;

                switch(topology)
                {
                    case(VK_PRIMITIVE_TOPOLOGY_POINT_LIST):
                    {
                        uint32_t endVertex = firstVertex + vertexCount;
                        for (uint32_t i = firstVertex; i < endVertex; ++i)
                        {
                            T::point(i);
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST):
                    {
                        uint32_t primitiveCount = vertexCount / 2;
                        uint32_t endVertex = firstVertex + primitiveCount * 2;
                        for (uint32_t i = firstVertex; i < endVertex; i += 2)
                        {
                            T::line(i, i+1);
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP):
                    {
                        uint32_t endVertex = firstVertex + ((vertexCount >= 2) ? (vertexCount-1) : 0);
                        for (uint32_t i = firstVertex; i < endVertex; ++i)
                        {
                            T::line(i, i+1);
                        }
                        break;
                    }
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
                    {
                        uint32_t endVertex = firstVertex + ((vertexCount >= 3) ? (vertexCount-2) : 0);
                        for (uint32_t i = firstVertex; i < endVertex; ++i)
                        {
                            T::triangle(i, i+1, i+2); // do we need to reverse the i+1 and i+2 order on odd triangles?
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN):
                    {
                        uint32_t endVertex = firstVertex + ((vertexCount >= 3) ? (vertexCount-2) : 0);
                        for (uint32_t i = firstVertex+1; i < endVertex; ++i)
                        {
                            T::triangle(firstVertex, i+1, i+2);
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST):
                    default:
                        warn("PrimitiveFunctor::draw(topology = ", topology, ", ...) not implemented.");
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
                if (!T::instance(instanceIndex)) continue;

                switch(topology)
                {
                    case(VK_PRIMITIVE_TOPOLOGY_POINT_LIST):
                    {
                        uint32_t endIndex = firstIndex + indexCount;
                        for (uint32_t i = firstIndex; i < endIndex; ++i)
                        {
                            T::point(indices->at(i));
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST):
                    {
                        uint32_t primtiveCount = indexCount / 2;
                        uint32_t endIndex = firstIndex + primtiveCount * 2;
                        for (uint32_t i = firstIndex; i < endIndex; i += 2)
                        {
                            T::line(indices->at(i), indices->at(i + 1));
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP):
                    {
                        uint32_t endIndex= firstIndex + ((indexCount >= 2) ? (indexCount-1) : 0);
                        for (uint32_t i = firstIndex; i < endIndex; ++i)
                        {
                            T::line(indices->at(i), indices->at(i + 1));
                        }
                        break;
                    }
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
                    {
                        uint32_t endIndex = firstIndex + ((indexCount >= 3) ? (indexCount-2) : 0);
                        for (uint32_t i = firstIndex; i < endIndex; ++i)
                        {
                            T::triangle(indices->at(i), indices->at(i+1), indices->at(i+2)); // do we need to reverse the i+1 and i+2 order on odd triangles?
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN):
                    {
                        uint32_t endIndex = firstIndex + ((indexCount >= 3) ? (indexCount-2) : 0);
                        for (uint32_t i = firstIndex+1; i < endIndex; ++i)
                        {
                            T::triangle(indices->at(firstIndex), indices->at(i+1), indices->at(i+2));
                        }
                        break;
                    }
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY):
                    case(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST):
                    default:
                        warn("PrimitiveFunctor::drawIndexed(topology = ", topology, ", ...) not implemented.");
                        break;
                }
            }
        }
    };


} // namespace vsg
