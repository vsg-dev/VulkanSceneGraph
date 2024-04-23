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
    class Layer;
    class Transform;
    class MatrixTransform;
    class Joint;
    class TileDatabase;
    class VertexDraw;
    class VertexIndexDraw;
    class Geometry;
    class Command;
    class StateCommand;
    class Commands;
    class CommandBuffer;
    class State;
    class DatabasePager;
    class FrameStamp;
    class CulledPagedLODs;
    class View;
    class Bin;
    class Switch;
    class RegionOfInterest;
    class ViewDependentState;
    class Light;
    class AmbientLight;
    class DirectionalLight;
    class PointLight;
    class SpotLight;
    class CommandGraph;
    class RecordedCommandBuffers;
    class Instrumentation;

    VSG_type_name(vsg::RecordTraversal);

    /// RecordTraversal traverses a scene graph doing view frustum culling and invoking state/commands to record them to a Vulkan command buffer
    class VSG_DECLSPEC RecordTraversal : public Object
    {
    public:
        explicit RecordTraversal(uint32_t in_maxSlot = 2, std::set<Bin*> in_bins = {});

        RecordTraversal(const RecordTraversal&) = delete;
        RecordTraversal& operator=(const RecordTraversal& rhs) = delete;

        template<typename... Args>
        static ref_ptr<RecordTraversal> create(Args&&... args)
        {
            return ref_ptr<RecordTraversal>(new RecordTraversal(std::forward<Args>(args)...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(RecordTraversal); }
        const char* className() const noexcept override { return type_name<RecordTraversal>(); }

        Mask traversalMask = MASK_ALL;
        Mask overrideMask = MASK_OFF;

        ref_ptr<Instrumentation> instrumentation;

        /// Container for CommandBuffers that have been recorded in current frame
        ref_ptr<RecordedCommandBuffers> recordedCommandBuffers;

        /// get the current State object used to track state and projection/modelview matrices for the current subgraph being traversed
        State* getState() { return _state; }

        /// get the current CommandBuffer for the current subgraph being traversed
        CommandBuffer* getCommandBuffer();

        /// get the current DeviceID for the current subgraph being traversed
        uint32_t deviceID() const;

        void setFrameStamp(FrameStamp* fs);
        FrameStamp* getFrameStamp() { return _frameStamp; }

        void setDatabasePager(DatabasePager* dp);
        DatabasePager* getDatabasePager() { return _databasePager; }

        void apply(const Object& object);

        // scene graph nodes
        void apply(const Group& group);
        void apply(const QuadGroup& quadGroup);
        void apply(const LOD& lod);
        void apply(const PagedLOD& pagedLOD);
        void apply(const TileDatabase& tileDatabase);
        void apply(const CullGroup& cullGroup);
        void apply(const CullNode& cullNode);
        void apply(const DepthSorted& depthSorted);
        void apply(const Layer& layer);
        void apply(const Switch& sw);
        void apply(const RegionOfInterest& roi);

        // leaf node
        void apply(const VertexDraw& vid);
        void apply(const VertexIndexDraw& vid);
        void apply(const Geometry& vid);

        // positional state
        void apply(const Light& light);
        void apply(const AmbientLight& light);
        void apply(const DirectionalLight& light);
        void apply(const PointLight& light);
        void apply(const SpotLight& light);

        // transform nodes
        void apply(const Transform& transform);
        void apply(const MatrixTransform& mt);

        // Animation nodes
        void apply(const Joint& joint);

        // Vulkan nodes
        void apply(const StateGroup& object);

        // Commands
        void apply(const Commands& commands);
        void apply(const Command& command);
        void apply(const StateCommand&);

        // Viewer level nodes
        void apply(const Bin& bin);
        void apply(const View& view);
        void apply(const CommandGraph& commandGraph);

        void addToBin(int32_t binNumber, double value, const Node* node);

        // clear the bins to record a new frame.
        void clearBins();

        // list of pairs of modelview matrix & region of interest
        std::vector<std::pair<dmat4, const RegionOfInterest*>> regionsOfInterest;

    protected:
        virtual ~RecordTraversal();

        ref_ptr<FrameStamp> _frameStamp;
        ref_ptr<State> _state;

        // used to handle loading of PagedLOD external children.
        ref_ptr<DatabasePager> _databasePager;
        ref_ptr<CulledPagedLODs> _culledPagedLODs;

        int32_t _minimumBinNumber = 0;
        std::vector<ref_ptr<Bin>> _bins;
        ref_ptr<ViewDependentState> _viewDependentState;
    };

} // namespace vsg
