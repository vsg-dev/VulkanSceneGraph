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
#include <vsg/traversals/ComputeBounds.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/vk/ResourceRequirements.h>

#include "shaders/simple_tile_frag.cpp"
#include "shaders/simple_tile_vert.cpp"

using namespace vsg;

tile::tile()
{
}

void tile::read(vsg::Input& input)
{
    input.readObject("settings", settings);

    init(input.options);
}

void tile::write(vsg::Output& output) const
{
    output.writeObject("settings", settings);
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
    auto replace = [](vsg::Path& path, const std::string& match, uint32_t value) {
        std::stringstream sstr;
        sstr << value;
        auto levelPos = path.find(match);
        if (levelPos != vsg::Path::npos) path.replace(levelPos, match.length(), sstr.str());
    };

    vsg::Path path = src;
    replace(path, "{z}", level);
    replace(path, "{x}", x);
    replace(path, "{y}", y);

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

        vsg::debug("read(", filename, ") -> tile_info = ", tile_info, ", x = ", x, ", y = ", y, ", z = ", lod);

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
            //auto terrainPath = getTilePath(terrainLayer, x, y, lod);

            auto imageTile = vsg::read_cast<vsg::Data>(imagePath, options);
            //auto terrainTile = vsg::read(terrainPath, options);

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
                    plod->options = options;

                    group->addChild(plod);
                }
            }
        }
    }

    uint32_t maxLevel = 20;
    uint32_t estimatedNumOfTilesBelow = 0;
    uint32_t maxNumTilesBelow = 1024;

    uint32_t level = 0;
    for (uint32_t i = level; i < maxLevel; ++i)
    {
        estimatedNumOfTilesBelow += std::pow(4, i - level);
    }

    uint32_t tileMultiplier = std::min(estimatedNumOfTilesBelow, maxNumTilesBelow) + 1;

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
            auto& tileID = pathToTileID[tilePath];
            auto imageTile = object.cast<vsg::Data>();
            if (imageTile)
            {
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
                        plod->options = options;

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
        vsg::warn("Could not load all 4 subtiles, loaded only ", group->children.size(), " tiles.");

        return {};
    }

    return group;
}

void tile::init(vsg::ref_ptr<const vsg::Options> options)
{
    if (!descriptorSetLayout)
    {
        vsg::DescriptorSetLayoutBindings descriptorBindings{
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
        };

        descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);
    }

    if (!pipelineLayout)
    {
        vsg::PushConstantRanges pushConstantRanges{
            {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls autoaatically provided by the VSG's DispatchTraversal
        };

        pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    }

    if (!sampler)
    {
        sampler = vsg::Sampler::create();
        sampler->maxLod = settings->mipmapLevelsHint;
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->anisotropyEnable = VK_TRUE;
        sampler->maxAnisotropy = 16.0f;
    }

    if (!graphicsPipeline)
    {
        auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/simple_tile.vert", options);
        if (!vertexShader) vertexShader = simple_tile_vert(); // fallback to shaders/simple_tile_vert.cpp

        auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/simple_tile.frag", options);
        if (!fragmentShader) fragmentShader = simple_tile_frag(); // fallback to shaders/simple_tile_frag.cpp

        if (!vertexShader || !fragmentShader)
        {
            vsg::error("Could not create shaders.");
        }

        vsg::VertexInputState::Bindings vertexBindingsDescriptions{
            VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
            VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
            VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
        };

        vsg::VertexInputState::Attributes vertexAttributeDescriptions{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
            VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // colour data
            VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},    // tex coord data
        };

        vsg::GraphicsPipelineStates pipelineStates{
            vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
            vsg::InputAssemblyState::create(),
            vsg::RasterizationState::create(),
            vsg::MultisampleState::create(),
            vsg::ColorBlendState::create(),
            vsg::DepthStencilState::create()};

        graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    }
}

vsg::ref_ptr<vsg::StateGroup> tile::createRoot() const
{
    auto root = vsg::StateGroup::create();
    root->add(vsg::BindGraphicsPipeline::create(graphicsPipeline));

    return root;
}

vsg::ref_ptr<vsg::Node> tile::createTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> sourceData) const
{
#if 1
    return createECEFTile(tile_extents, sourceData);
#else
    return createTextureQuad(tile_extents, sourceData);
#endif
}

vsg::ref_ptr<vsg::Node> tile::createECEFTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> textureData) const
{
    vsg::dvec3 center = computeLatitudeLongitudeAltitude((tile_extents.min + tile_extents.max) * 0.5);

    auto localToWorld = settings->ellipsoidModel->computeLocalToWorldTransform(center);
    auto worldToLocal = vsg::inverse(localToWorld);

    // create texture image and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, vsg::DescriptorSets{descriptorSet});

    // create StateGroup to bind any texture state
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindDescriptorSets);

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
    if (textureData->getLayout().origin == vsg::TOP_LEFT)
    {
        tCoordScale = -tCoordScale;
        tCoordOrigin = 1.0f;
    }

    vsg::vec3 color(1.0f, 1.0f, 1.0f);

    // set up vertex coords
    auto vertices = vsg::vec3Array::create(numVertices);
    auto colors = vsg::vec3Array::create(numVertices);
    auto texcoords = vsg::vec2Array::create(numVertices);
    for (uint32_t r = 0; r < numRows; ++r)
    {
        for (uint32_t c = 0; c < numCols; ++c)
        {
            vsg::dvec3 location(longitudeOrigin + double(c) * longitudeScale, latitudeOrigin + double(r) * latitudeScale, 0.0);
            vsg::dvec3 latitudeLongitudeAltitude = computeLatitudeLongitudeAltitude(location);

            auto ecef = settings->ellipsoidModel->convertLatLongAltitudeToECEF(latitudeLongitudeAltitude);
            vsg::vec3 vertex(worldToLocal * ecef);
            vsg::vec2 texcoord(float(c) * sCoordScale, tCoordOrigin + float(r) * tCoordScale);

            uint32_t vi = c + r * numCols;
            vertices->set(vi, vertex);
            colors->set(vi, color);
            texcoords->set(vi, texcoord);
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
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vertices, colors, texcoords}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    drawCommands->addChild(vsg::DrawIndexed::create(indices->size(), 1, 0, 0, 0));

    // add drawCommands to transform
    transform->addChild(drawCommands);

    return scenegraph;
}

vsg::ref_ptr<vsg::Node> tile::createTextureQuad(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> textureData) const
{
    if (!textureData) return {};

    // create texture image and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, vsg::DescriptorSets{descriptorSet});

    // create StateGroup to bind any texture state
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindDescriptorSets);

    // set up model transformation node
    auto transform = vsg::MatrixTransform::create(); // VK_SHADER_STAGE_VERTEX_BIT

    // add transform to root of the scene graph
    scenegraph->addChild(transform);

    // set up vertex and index arrays
    float min_x = tile_extents.min.x;
    float min_y = tile_extents.min.y;
#if 1
    float max_x = tile_extents.max.x;
    float max_y = tile_extents.max.y;
#else
    float max_x = tile_extents.min.x * 0.05 + tile_extents.max.x * 0.95;
    float max_y = tile_extents.min.y * 0.05 + tile_extents.max.y * 0.95;
#endif

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

    uint8_t origin = textureData->getLayout().origin; // in Vulkan the origin is by default top left.
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
