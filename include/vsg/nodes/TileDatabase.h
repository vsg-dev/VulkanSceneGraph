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

    /// TileDatabaseSettings provides the settings used by vsg::TileDatabase and vsg::tile ReaderWriter to guide paging in image, DEM tiles from disk/http/.
    class VSG_DECLSPEC TileDatabaseSettings : public Inherit<Object, TileDatabaseSettings>
    {
    public:
        TileDatabaseSettings();
        TileDatabaseSettings(const TileDatabaseSettings& rhs, const CopyOp& copyop = {});

        // defaults for readymap
        dbox extents = {{-180.0, -90.0, 0.0}, {180.0, 90.0, 1.0}};
        uint32_t noX = 2;
        uint32_t noY = 1;
        uint32_t maxLevel = 22;
        bool originTopLeft = true;
        double lodTransitionScreenHeightRatio = 0.25;

        std::string projection;
        ref_ptr<EllipsoidModel> ellipsoidModel = EllipsoidModel::create();

        // callback for post processing loading image, detail or terrain layers data after the source data is loaded.
        using ProcessCallback = std::function<ref_ptr<Data>(ref_ptr<Data>)>;

        Path imageLayer;
        ProcessCallback imageLayerCallback;

        Path detailLayer;
        ProcessCallback detailLayerCallback;

        Path elevationLayer;
        ProcessCallback elevationLayerCallback;
        double elevationScale = 32868.0;
        double skirtRatio = 0.02;
        uint32_t maxTileDimension = 1024;

        uint32_t mipmapLevelsHint = 16;

        /// hint of whether to use flat shaded shaders or with lighting enabled.
        bool lighting = true;

        /// optional shaderSet to use for setting up shaders, if left null use vsg::createTileShaderSet().
        ref_ptr<ShaderSet> shaderSet;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return TileDatabaseSettings::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        // read/write of TileReader settings
        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::TileDatabaseSettings);

    /// TileDatabase node is the root node of a paged tile database graph that is automatically populated
    /// by the vsg::tile ReaderWriter using the TileDatabaseSettings as a guide to the source of data and sizing
    class VSG_DECLSPEC TileDatabase : public Inherit<Node, TileDatabase>
    {
    public:
        TileDatabase();
        TileDatabase(const TileDatabase& rhs, const CopyOp& copyop = {});

        ref_ptr<TileDatabaseSettings> settings;
        ref_ptr<Node> child;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return TileDatabase::create(*this, copyop); }
        int compare(const Object& rhs) const override;

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

    /// convenience function for getting the part of a string enclosed between a start_match and end_match string
    extern VSG_DECLSPEC std::string_view find_field(const std::string& source, const std::string_view& start_match, const std::string_view& end_match);

    /// convenience function for replacing all instances of a match string with the replacement string.
    extern VSG_DECLSPEC void replace(std::string& source, const std::string_view& match, const std::string_view& replacement);

    /// convenience function for creating a TileDatabaseSettings for reading Bing Maps imagery
    /// Bing Maps official documentation:
    ///    metadata (includes imagerySet details): https://learn.microsoft.com/en-us/bingmaps/rest-services/imagery/get-imagery-metadata
    ///    culture codes: https://learn.microsoft.com/en-us/bingmaps/rest-services/common-parameters-and-types/supported-culture-codes
    ///    api key: https://www.microsoft.com/en-us/maps/create-a-bing-maps-key
    extern VSG_DECLSPEC ref_ptr<TileDatabaseSettings> createBingMapsSettings(const std::string& imagerySet, const std::string& culture, const std::string& key, ref_ptr<const Options> options);

    /// convenience function for creating a TileDatabaseSettings for reading OpenStreetMap imagery
    extern VSG_DECLSPEC ref_ptr<TileDatabaseSettings> createOpenStreetMapSettings(ref_ptr<const Options> options);

} // namespace vsg
