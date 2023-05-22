/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/io/tile.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/TileDatabase.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/state/material.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/vk/ResourceRequirements.h>

using namespace vsg;

tile::tile(ref_ptr<TileDatabaseSettings> in_settings, ref_ptr<const Options> in_options) :
    settings(in_settings)
{
    init(in_options);
}

vsg::dvec3 tile::computeLatitudeLongitudeAltitude(const vsg::dvec3& src) const
{
    if (settings->projection == "EPSG:3857" || settings->projection == "spherical-mercator")
    {
        double n = 2.0 * vsg::radians(src.y);
        double adjustedLatitude = vsg::degrees(atan(0.5 * (exp(n) - exp(-n))));
        return vsg::dvec3(adjustedLatitude, src.x, src.z);
    }
    else
    {
        return vsg::dvec3(src.y, src.x, src.z);
    }
}

vsg::dbox tile::computeTileExtents(uint32_t x, uint32_t y, uint32_t level) const
{
    double multiplier = pow(0.5, double(level));
    double tileWidth = multiplier * (settings->extents.max.x - settings->extents.min.x) / double(settings->noX);
    double tileHeight = multiplier * (settings->extents.max.y - settings->extents.min.y) / double(settings->noY);

    vsg::dbox tile_extents;
    if (settings->originTopLeft)
    {
        vsg::dvec3 origin(settings->extents.min.x, settings->extents.max.y, settings->extents.min.z);
        tile_extents.min = origin + vsg::dvec3(double(x) * tileWidth, -double(y + 1) * tileHeight, 0.0);
        tile_extents.max = origin + vsg::dvec3(double(x + 1) * tileWidth, -double(y) * tileHeight, 1.0);
    }
    else
    {
        tile_extents.min = settings->extents.min + vsg::dvec3(double(x) * tileWidth, double(y) * tileHeight, 0.0);
        tile_extents.max = settings->extents.min + vsg::dvec3(double(x + 1) * tileWidth, double(y + 1) * tileHeight, 1.0);
    }
    return tile_extents;
}

vsg::Path tile::getTilePath(const vsg::Path& src, uint32_t x, uint32_t y, uint32_t level) const
{
    auto replace = [](Path& path, const std::string& match, uint32_t value) {
        std::stringstream sstr;
        sstr << value;
        auto levelPos = path.find(match);
        if (levelPos != Path::npos) path.replace(levelPos, match.length(), sstr.str());
    };

    vsg::Path path = src;
    if (auto quadkeyPos = path.find("{quadkey}"); quadkeyPos != Path::npos)
    {
        std::string quadkey;
        uint32_t mask = 1 << level;
        for (uint32_t i = level + 1; i > 0; --i)
        {
            char digit = '0';
            if ((x & mask) != 0) digit += 1;
            if ((y & mask) != 0) digit += 2;
            quadkey.push_back(digit);
            mask = mask >> 1;
        }

        path.replace(quadkeyPos, 9, quadkey); // 9 is char length of {quadkey}
    }
    else
    {
        replace(path, "{z}", level);
        replace(path, "{x}", x);
        replace(path, "{y}", y);
    }

    return path;
}

vsg::ref_ptr<vsg::Object> tile::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    auto extension = vsg::lowerCaseFileExtension(filename);
    if (extension != ".tile") return {};

    auto tile_info = filename.substr(0, filename.length() - 5);
    if (tile_info == "root")
    {
        return read_root(options);
    }
    else
    {
        std::basic_stringstream<vsg::Path::value_type> sstr(tile_info);

        uint32_t x, y, lod;
        sstr >> x >> y >> lod;

        vsg::debug("read(", filename, ") -> tile_info = ", tile_info, ", x = ", x, ", y = ", y, ", z = ", lod, ", tile = ", this, ", settings =  ", settings);

        return read_subtile(x, y, lod, options);
    }
}

vsg::ref_ptr<vsg::Object> tile::read_root(vsg::ref_ptr<const vsg::Options> options) const
{
    auto group = createRoot();

    uint32_t lod = 0;
    for (uint32_t y = 0; y < settings->noY; ++y)
    {
        for (uint32_t x = 0; x < settings->noX; ++x)
        {
            auto imagePath = getTilePath(settings->imageLayer, x, y, lod);
            auto imageTile = vsg::read_cast<vsg::Data>(imagePath, options);

            if (imageTile)
            {
                auto tile_extents = computeTileExtents(x, y, lod);
                auto tile_node = createTile(tile_extents, imageTile);
                if (tile_node)
                {
                    vsg::ComputeBounds computeBound;
                    tile_node->accept(computeBound);
                    auto& bb = computeBound.bounds;
                    vsg::dsphere bound((bb.min.x + bb.max.x) * 0.5, (bb.min.y + bb.max.y) * 0.5, (bb.min.z + bb.max.z) * 0.5, vsg::length(bb.max - bb.min) * 0.5);

                    auto plod = vsg::PagedLOD::create();
                    plod->bound = bound;
                    plod->children[0] = vsg::PagedLOD::Child{0.25, {}};       // external child visible when it's bound occupies more than 1/4 of the height of the window
                    plod->children[1] = vsg::PagedLOD::Child{0.0, tile_node}; // visible always
                    plod->filename = vsg::make_string(x, " ", y, " 0.tile");
                    plod->options = Options::create_if(options, *options);

                    group->addChild(plod);
                }
            }
        }
    }

    uint64_t maxLevel = 20;
    uint64_t estimatedNumOfTilesBelow = 0;
    uint64_t maxNumTilesBelow = 1024;
    for (uint64_t level = 0; level < maxLevel; ++level)
    {
        uint64_t num_tiles_at_level = 1ull << (2ull * (level));
        estimatedNumOfTilesBelow += num_tiles_at_level;
    }

    uint32_t tileMultiplier = static_cast<uint32_t>(std::min(estimatedNumOfTilesBelow, maxNumTilesBelow) + 1);
    // vsg::info("estimatedNumOfTilesBelow = ", estimatedNumOfTilesBelow, ", tileMultiplier = ", tileMultiplier);

    // set up the ResourceHints required to make sure the VSG preallocates enough Vulkan resources for the paged database
    vsg::CollectResourceRequirements collectResourceRequirements;
    group->accept(collectResourceRequirements);
    group->setObject("ResourceHints", collectResourceRequirements.createResourceHints(tileMultiplier));

    // assign the EllipsoidModel so that the overall geometry of the database can be used as guide for clipping and navigation.
    group->setObject("EllipsoidModel", settings->ellipsoidModel);

    return group;
}

vsg::ref_ptr<vsg::Object> tile::read_subtile(uint32_t x, uint32_t y, uint32_t lod, vsg::ref_ptr<const vsg::Options> options) const
{
    // need to load subtile x y lod

    vsg::time_point start_read = vsg::clock::now();

    auto group = vsg::Group::create();

    struct TileID
    {
        uint32_t local_x;
        uint32_t local_y;
    };

    vsg::Paths tiles;
    std::map<vsg::Path, TileID> pathToTileID;

    uint32_t subtile_x = x * 2;
    uint32_t subtile_y = y * 2;
    uint32_t local_lod = lod + 1;
    for (uint32_t dy = 0; dy < 2; ++dy)
    {
        for (uint32_t dx = 0; dx < 2; ++dx)
        {
            uint32_t local_x = subtile_x + dx;
            uint32_t local_y = subtile_y + dy;
            auto tilePath = getTilePath(settings->imageLayer, local_x, local_y, local_lod);
            tiles.push_back(tilePath);
            pathToTileID[tilePath] = TileID{local_x, local_y};
        }
    }

    auto pathObjects = vsg::read(tiles, options);

    if (pathObjects.size() == 4)
    {
        for (auto& [tilePath, object] : pathObjects)
        {
            auto imageTile = object.cast<vsg::Data>();
            if (imageTile)
            {
                auto& tileID = pathToTileID[tilePath];
                auto tile_extents = computeTileExtents(tileID.local_x, tileID.local_y, local_lod);
                auto tile_node = createTile(tile_extents, imageTile);
                if (tile_node)
                {
                    vsg::ComputeBounds computeBound;
                    tile_node->accept(computeBound);
                    auto& bb = computeBound.bounds;
                    vsg::dsphere bound((bb.min.x + bb.max.x) * 0.5, (bb.min.y + bb.max.y) * 0.5, (bb.min.z + bb.max.z) * 0.5, vsg::length(bb.max - bb.min) * 0.5);

                    if (local_lod < settings->maxLevel)
                    {
                        auto plod = vsg::PagedLOD::create();
                        plod->bound = bound;
                        plod->children[0] = vsg::PagedLOD::Child{settings->lodTransitionScreenHeightRatio, {}}; // external child visible when it's bound occupies more than 1/4 of the height of the window
                        plod->children[1] = vsg::PagedLOD::Child{0.0, tile_node};                               // visible always
                        plod->filename = vsg::make_string(tileID.local_x, " ", tileID.local_y, " ", local_lod, ".tile");
                        plod->options = Options::create_if(options, *options);

                        vsg::debug("plod->filename ", plod->filename);

                        group->addChild(plod);
                    }
                    else
                    {
                        auto cullGroup = vsg::CullGroup::create();
                        cullGroup->bound = bound;
                        cullGroup->addChild(tile_node);

                        group->addChild(cullGroup);
                    }
                }
            }
        }
    }

    vsg::time_point end_read = vsg::clock::now();

    double time_to_read_tile = std::chrono::duration<float, std::chrono::milliseconds::period>(end_read - start_read).count();

    {
        std::scoped_lock<std::mutex> lock(statsMutex);
        numTilesRead += 1;
        totalTimeReadingTiles += time_to_read_tile;
    }

    if (group->children.size() != 4)
    {
        return ReadError::create("vsg::tile::read_subtile(..) could not load all 4 subtiles.");
    }

    return group;
}

void tile::init(vsg::ref_ptr<const vsg::Options> options)
{
    if (settings->shaderSet)
    {
        _shaderSet = settings->shaderSet;
    }
    else
    {
        if (settings->lighting)
            _shaderSet = createPhongShaderSet(options);
        else
            _shaderSet = createFlatShadedShaderSet(options);
    }

    _sampler = vsg::Sampler::create();
    _sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    _sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    _sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    _sampler->anisotropyEnable = VK_TRUE;
    _sampler->maxAnisotropy = 16.0f;
    _sampler->maxLod = static_cast<float>(settings->mipmapLevelsHint);

    _graphicsPipelineConfig = GraphicsPipelineConfigurator::create(_shaderSet);

    if (settings->imageLayer)
    {
        _graphicsPipelineConfig->enableTexture("diffuseMap");
    }
#if 0
    if (settings->terrainLayer)
    {
        _graphicsPipelineConfig->enableTexture("displacementMap");
    }
#endif
    _graphicsPipelineConfig->enableUniform("material");

    _graphicsPipelineConfig->enableArray("vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, 12, VK_FORMAT_R32G32B32_SFLOAT);
    _graphicsPipelineConfig->enableArray("vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX, 12, VK_FORMAT_R32G32B32_SFLOAT);
    _graphicsPipelineConfig->enableArray("vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, 8, VK_FORMAT_R32G32_SFLOAT);
    _graphicsPipelineConfig->enableArray("vsg_Color", VK_VERTEX_INPUT_RATE_INSTANCE, 16, VK_FORMAT_R32G32B32A32_SFLOAT);

    if (settings->lighting)
    {
        auto vdsl = ViewDescriptorSetLayout::create();
        _graphicsPipelineConfig->additionalDescriptorSetLayout = vdsl;
    }

    if (auto& materialBinding = _shaderSet->getUniformBinding("material"))
    {
        ref_ptr<Data> mat = materialBinding.data;
        if (!mat) mat = vsg::PhongMaterialValue::create();
        _material = vsg::DescriptorBuffer::create(mat, materialBinding.binding, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }
    else
    {
        auto mat = vsg::PhongMaterialValue::create();
        _material = vsg::DescriptorBuffer::create(mat, 10, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }

    _graphicsPipelineConfig->init();
}

vsg::ref_ptr<vsg::StateGroup> tile::createRoot() const
{
    auto root = vsg::StateGroup::create();
    root->add(_graphicsPipelineConfig->bindGraphicsPipeline);

    if (settings->lighting)
    {
        auto bindViewDescriptorSets = BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipelineConfig->layout, 1);
        root->add(bindViewDescriptorSets);
    }

    return root;
}

vsg::ref_ptr<vsg::Node> tile::createTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> sourceData) const
{
    if (settings->ellipsoidModel)
    {
        return createECEFTile(tile_extents, sourceData);
    }
    else
    {
        return createTextureQuad(tile_extents, sourceData);
    }
}

vsg::ref_ptr<vsg::Node> tile::createECEFTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> textureData) const
{
    vsg::dvec3 center = computeLatitudeLongitudeAltitude((tile_extents.min + tile_extents.max) * 0.5);

    auto localToWorld = settings->ellipsoidModel->computeLocalToWorldTransform(center);
    auto worldToLocal = vsg::inverse(localToWorld);

    vsg::dmat3 normalMatrix(localToWorld(0, 0), localToWorld(0, 1), localToWorld(0, 2),
                            localToWorld(1, 0), localToWorld(1, 1), localToWorld(1, 2),
                            localToWorld(2, 0), localToWorld(2, 1), localToWorld(2, 2));

    // create texture image, material and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(_sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipelineConfig->layout, 0, vsg::Descriptors{texture, _material});

    // create StateGroup to bind any texture state
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindDescriptorSet);

    // set up model transformation node
    auto transform = vsg::MatrixTransform::create(localToWorld); // VK_SHADER_STAGE_VERTEX_BIT

    // add transform to root of the scene graph
    scenegraph->addChild(transform);

    uint32_t numRows = 32;
    uint32_t numCols = 32;
    uint32_t numVertices = numRows * numCols;
    uint32_t numTriangles = (numRows - 1) * (numCols - 1) * 2;

    double longitudeOrigin = tile_extents.min.x;
    double longitudeScale = (tile_extents.max.x - tile_extents.min.x) / double(numCols - 1);
    double latitudeOrigin = tile_extents.min.y;
    double latitudeScale = (tile_extents.max.y - tile_extents.min.y) / double(numRows - 1);

    float sCoordScale = 1.0f / float(numCols - 1);
    float tCoordScale = 1.0f / float(numRows - 1);
    float tCoordOrigin = 0.0;
    if (textureData->properties.origin == vsg::TOP_LEFT)
    {
        tCoordScale = -tCoordScale;
        tCoordOrigin = 1.0f;
    }

    vsg::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

    // set up vertex coords
    auto vertices = vsg::vec3Array::create(numVertices);
    auto normals = vsg::vec3Array::create(numVertices);
    auto texcoords = vsg::vec2Array::create(numVertices);
    auto colors = vsg::vec4Value::create(color);
    for (uint32_t r = 0; r < numRows; ++r)
    {
        for (uint32_t c = 0; c < numCols; ++c)
        {
            vsg::dvec3 location(longitudeOrigin + double(c) * longitudeScale, latitudeOrigin + double(r) * latitudeScale, 0.0);
            vsg::dvec3 latitudeLongitudeAltitude = computeLatitudeLongitudeAltitude(location);

            auto ecef = settings->ellipsoidModel->convertLatLongAltitudeToECEF(latitudeLongitudeAltitude);
            vsg::vec3 vertex(worldToLocal * ecef);
            vsg::vec3 normal(normalize(ecef * normalMatrix));
            vsg::vec2 texcoord(float(c) * sCoordScale, tCoordOrigin + float(r) * tCoordScale);

            uint32_t vi = c + r * numCols;
            vertices->set(vi, vertex);
            texcoords->set(vi, texcoord);
            normals->set(vi, normal);
        }
    }

    // set up indices
    auto indices = vsg::ushortArray::create(numTriangles * 3);
    auto itr = indices->begin();
    for (uint32_t r = 0; r < numRows - 1; ++r)
    {
        for (uint32_t c = 0; c < numCols - 1; ++c)
        {
            uint32_t vi = c + r * numCols;
            (*itr++) = vi;
            (*itr++) = vi + 1;
            (*itr++) = vi + numCols;
            (*itr++) = vi + numCols;
            (*itr++) = vi + 1;
            (*itr++) = vi + numCols + 1;
        }
    }

    // setup geometry
    auto drawCommands = vsg::Commands::create();
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vertices, normals, texcoords, colors}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    drawCommands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->size()), 1, 0, 0, 0));

    // add drawCommands to transform
    transform->addChild(drawCommands);

    return scenegraph;
}

vsg::ref_ptr<vsg::Node> tile::createTextureQuad(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> textureData) const
{
    if (!textureData) return {};

    // create texture image and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(_sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipelineConfig->layout, 0, vsg::Descriptors{texture});

    // create StateGroup to bind any texture state
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindDescriptorSet);

    // set up model transformation node
    auto transform = vsg::MatrixTransform::create();

    // add transform to root of the scene graph
    scenegraph->addChild(transform);

    // set up vertex and index arrays
    float min_x = static_cast<float>(tile_extents.min.x);
    float min_y = static_cast<float>(tile_extents.min.y);
    float max_x = static_cast<float>(tile_extents.max.x);
    float max_y = static_cast<float>(tile_extents.max.y);

    auto vertices = vsg::vec3Array::create(
        {{min_x, 0.0f, min_y},
         {max_x, 0.0f, min_y},
         {max_x, 0.0f, max_y},
         {min_x, 0.0f, max_y}}); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_INSTANCE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    auto colors = vsg::vec3Array::create(
        {{1.0f, 1.0f, 1.0f},
         {1.0f, 1.0f, 1.0f},
         {1.0f, 1.0f, 1.0f},
         {1.0f, 1.0f, 1.0f}}); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    uint8_t origin = textureData->properties.origin; // in Vulkan the origin is by default top left.
    float left = 0.0f;
    float right = 1.0f;
    float top = (origin == vsg::TOP_LEFT) ? 0.0f : 1.0f;
    float bottom = (origin == vsg::TOP_LEFT) ? 1.0f : 0.0f;
    auto texcoords = vsg::vec2Array::create(
        {{left, bottom},
         {right, bottom},
         {right, top},
         {left, top}}); // VK_FORMAT_R32G32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    auto indices = vsg::ushortArray::create(
        {0, 1, 2,
         2, 3, 0}); // VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    // setup geometry
    auto drawCommands = vsg::Commands::create();
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vertices, colors, texcoords}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    drawCommands->addChild(vsg::DrawIndexed::create(6, 1, 0, 0, 0));

    // add drawCommands to transform
    transform->addChild(drawCommands);

    return scenegraph;
}
