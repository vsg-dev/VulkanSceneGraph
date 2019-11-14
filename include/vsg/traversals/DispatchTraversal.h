#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <memory>
#include <vsg/core/Object.h>
#include <vsg/maths/mat4.h>

namespace vsg
{

    // forward declare nodes
    class Node;
    class Group;
    class QuadGroup;
    class LOD;
    class PagedLOD;
    class StateGroup;
    class CullGroup;
    class CullNode;
    class MatrixTransform;
    class Command;
    class Commands;
    class CommandBuffer;
    class State;
    class DatabasePager;
    class FrameStamp;
    class CulledPagedLODs;

    class VSG_DECLSPEC DispatchTraversal : public Object
    {
    public:
        explicit DispatchTraversal(CommandBuffer* commandBuffer = nullptr, uint32_t maxSlot = 2, ref_ptr<FrameStamp> fs = {});
        ~DispatchTraversal();

        void setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix);

        void apply(const Object& object);

        // scene graph nodes
        void apply(const Group& group);
        void apply(const QuadGroup& quadGrouo);
        void apply(const LOD& lod);
        void apply(const PagedLOD& pagedLOD);
        void apply(const CullGroup& cullGroup);
        void apply(const CullNode& cullNode);

        // Vulkan nodes
        void apply(const MatrixTransform& mt);
        void apply(const StateGroup& object);
        void apply(const Commands& commands);
        void apply(const Command& command);

        // used to handle loading of PagedLOD external children.
        ref_ptr<DatabasePager> databasePager;
        ref_ptr<CulledPagedLODs> culledPagedLODs;

        ref_ptr<FrameStamp> frameStamp;
        ref_ptr<State> state;
    };
} // namespace vsg
