#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/EllipsoidModel.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/Node.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/Sampler.h>
#include <vsg/utils/ShaderSet.h>

namespace vsg
{

    /// TileDatabaseSettings provides the settings used by vsg::TileDatabase & vsg::tile ReaderWriter to guide paging in image, DEM tiles from disk/http/.
    class VSG_DECLSPEC TileDatabaseSettings : public Inherit<Object, TileDatabaseSettings>
    {
    public:
        // read/write of TileReader settings
        void read(Input& input) override;
        void write(Output& output) const override;

        // defaults for readymap
        dbox extents = {{-180.0, -90.0, 0.0}, {180.0, 90.0, 1.0}};
        uint32_t noX = 2;
        uint32_t noY = 1;
        uint32_t maxLevel = 22;
        bool originTopLeft = true;
        double lodTransitionScreenHeightRatio = 0.25;

        std::string projection;
        ref_ptr<EllipsoidModel> ellipsoidModel = EllipsoidModel::create();

        Path imageLayer;
        Path terrainLayer;
        uint32_t mipmapLevelsHint = 16;

        /// hint of whether to use flat shaded shaders or with lighting enabled.
        bool lighting = true;

        /// optional shaderSet to use for setting up shaders, if left null use vsg::createTileShaderSet().
        ref_ptr<ShaderSet> shaderSet;
    };
    VSG_type_name(vsg::TileDatabaseSettings);

    /// TileDatabase node is the root node of a paged tile database graph that is automatically populated
    /// by the vsg::tile ReaderWriter using the TileDatabaseSettings as a guide to the source of data and sizing
    class VSG_DECLSPEC TileDatabase : public Inherit<Node, TileDatabase>
    {
    public:
        ref_ptr<TileDatabaseSettings> settings;
        ref_ptr<Node> child;

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            if (node.child) node.child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        // read/write of TileReader settings
        void read(Input& input) override;
        void write(Output& output) const override;

        bool readDatabase(ref_ptr<const Options> options);
    };
    VSG_type_name(vsg::TileDatabase);

} // namespace vsg
