#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/TileDatabase.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/ShaderSet.h>

namespace vsg
{

    /// tile reader that is used by the vsg::TileDatabase node to implement the reading of external tiles
    class VSG_DECLSPEC tile : public Inherit<ReaderWriter, tile>
    {
    public:
        tile(ref_ptr<TileDatabaseSettings> in_settings, ref_ptr<const Options> in_options);

        tile(const tile&) = delete;
        tile& operator=(const tile&) = delete;

        /// settings provided by the vsg::TileDatabase node
        ref_ptr<TileDatabaseSettings> settings;

        /// read the tile
        ref_ptr<Object> read(const Path& filename, ref_ptr<const Options> options = {}) const override;

        // timing stats
        mutable std::mutex statsMutex;
        mutable uint64_t numTilesRead{0};
        mutable double totalTimeReadingTiles{0.0};

    protected:
        /// initialize internal data structures
        void init(ref_ptr<const Options> options);

        dvec3 computeLatitudeLongitudeAltitude(const dvec3& src) const;
        dbox computeTileExtents(uint32_t x, uint32_t y, uint32_t level) const;
        Path getTilePath(const Path& src, uint32_t x, uint32_t y, uint32_t level) const;

        ref_ptr<Object> read_root(ref_ptr<const Options> options = {}) const;
        ref_ptr<Object> read_subtile(uint32_t x, uint32_t y, uint32_t lod, ref_ptr<const Options> options = {}) const;

        ref_ptr<BindDescriptorSet> createBindDescriptorSet(ref_ptr<Data> imageData, ref_ptr<Data> detailData, ref_ptr<Data> elevationData, Origin& origin, const vec3& displacementMapScale) const;

        ref_ptr<Node> createTile(const dbox& tile_extents, ref_ptr<Data> imageData, ref_ptr<Data> detailData, ref_ptr<Data> elevationData) const;
        ref_ptr<Node> createECEFTile(const dbox& tile_extents, ref_ptr<Data> imageData, ref_ptr<Data> detailData, ref_ptr<Data> elevationData) const;
        ref_ptr<Node> createTextureQuad(const dbox& tile_extents, ref_ptr<Data> imageData, ref_ptr<Data> detailData, ref_ptr<Data> elevationData) const;

        ref_ptr<StateGroup> createRoot() const;

        ref_ptr<ShaderSet> _shaderSet;
        ref_ptr<GraphicsPipelineConfigurator> _graphicsPipelineConfig;
        uint32_t _materialSetIndex = 1;
        ref_ptr<Sampler> _sampler;
        ref_ptr<DescriptorBuffer> _material;

        ref_ptr<DescriptorImage> _imageFallback;
        ref_ptr<DescriptorImage> _detailFallback;
        ref_ptr<DescriptorImage> _elevationFallback;

        mutable std::mutex _geometryMapMutex;
        mutable std::map<dvec4, ref_ptr<VertexIndexDraw>> _geometryMap;
    };
    VSG_type_name(vsg::tile);

} // namespace vsg
