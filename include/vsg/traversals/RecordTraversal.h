#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Mask.h>
#include <vsg/core/Object.h>
#include <vsg/core/type_name.h>
#include <vsg/maths/mat4.h>

#include <set>
#include <vector>

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
    class DepthSorted;
    class Transform;
    class MatrixTransform;
    class Command;
    class Commands;
    class CommandBuffer;
    class State;
    class DatabasePager;
    class FrameStamp;
    class CulledPagedLODs;
    class View;
    class Bin;
    class Switch;
    class ViewDependentState;
    class Light;
    class AmbientLight;
    class DirectionalLight;
    class PointLight;
    class SpotLight;

    VSG_type_name(vsg::RecordTraversal);

    /// RecordTraversal traverses a scene graph doing view frustum culling and invoking state/commands to record them to Vulkan command buffer
    class VSG_DECLSPEC RecordTraversal : public Object
    {
    public:
        explicit RecordTraversal(CommandBuffer* in_commandBuffer = nullptr, uint32_t in_maxSlot = 2, std::set<Bin*> in_bins = {});

        RecordTraversal(const RecordTraversal&) = delete;
        RecordTraversal& operator=(const RecordTraversal& rhs) = delete;

        template<typename... Args>
        static ref_ptr<RecordTraversal> create(Args&&... args)
        {
            return ref_ptr<RecordTraversal>(new RecordTraversal(args...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(RecordTraversal); }
        const char* className() const noexcept override { return type_name<RecordTraversal>(); }

        Mask traversalMask = MASK_ALL;
        Mask overrideMask = MASK_OFF;

        State* getState() { return _state; }

        void setFrameStamp(FrameStamp* fs);
        FrameStamp* getFrameStamp() { return _frameStamp; }

        void setDatabasePager(DatabasePager* dp);
        DatabasePager* getDatabasePager() { return _databasePager; }

        void setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix);

        void apply(const Object& object);

        // scene graph nodes
        void apply(const Group& group);
        void apply(const QuadGroup& quadGrouo);
        void apply(const LOD& lod);
        void apply(const PagedLOD& pagedLOD);
        void apply(const CullGroup& cullGroup);
        void apply(const CullNode& cullNode);
        void apply(const DepthSorted& depthSorted);
        void apply(const Switch& sw);

        // positional state
        void apply(const Light& light);
        void apply(const AmbientLight& light);
        void apply(const DirectionalLight& light);
        void apply(const PointLight& light);
        void apply(const SpotLight& light);

        // Vulkan nodes
        void apply(const Transform& transform);
        void apply(const MatrixTransform& mt);
        void apply(const StateGroup& object);
        void apply(const Commands& commands);
        void apply(const Command& command);

        // Viewer level nodes
        void apply(const View& view);

        // clear the bins to record a new frame.
        void clearBins();

    protected:
        virtual ~RecordTraversal();

        FrameStamp* _frameStamp = nullptr;
        State* _state = nullptr;

        // used to handle loading of PagedLOD external children.
        DatabasePager* _databasePager = nullptr;
        CulledPagedLODs* _culledPagedLODs = nullptr;

        int32_t _minimumBinNumber = 0;
        std::vector<ref_ptr<Bin>> _bins;
        ref_ptr<ViewDependentState> _viewDependentState;
    };

} // namespace vsg
