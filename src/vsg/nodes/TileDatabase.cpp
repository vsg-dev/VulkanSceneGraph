/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/io/tile.h>
#include <vsg/nodes/TileDatabase.h>

using namespace vsg;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TileDatabaseSettings
//
void TileDatabaseSettings::read(vsg::Input& input)
{
    input.read("extents", extents);
    input.read("noX", noX);
    input.read("noY", noY);
    input.read("maxLevel", maxLevel);
    input.read("originTopLeft", originTopLeft);
    input.read("lodTransitionScreenHeightRatio", lodTransitionScreenHeightRatio);
    input.read("projection", projection);
    input.readObject("ellipsoidModel", ellipsoidModel);
    input.read("imageLayer", imageLayer);
    input.read("terrainLayer", terrainLayer);
    input.read("mipmapLevelsHint", mipmapLevelsHint);

    if (input.version_greater_equal(0, 7, 1))
    {
        input.read("lighting", lighting);
        input.readObject("shaderSet", shaderSet);
    }
}

void TileDatabaseSettings::write(vsg::Output& output) const
{
    output.write("extents", extents);
    output.write("noX", noX);
    output.write("noY", noY);
    output.write("maxLevel", maxLevel);
    output.write("originTopLeft", originTopLeft);
    output.write("lodTransitionScreenHeightRatio", lodTransitionScreenHeightRatio);
    output.write("projection", projection);
    output.writeObject("ellipsoidModel", ellipsoidModel);
    output.write("imageLayer", imageLayer);
    output.write("terrainLayer", terrainLayer);
    output.write("mipmapLevelsHint", mipmapLevelsHint);

    if (output.version_greater_equal(0, 7, 1))
    {
        output.write("lighting", lighting);
        output.writeObject("shaderSet", shaderSet);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TileDatabase
//
void TileDatabase::read(vsg::Input& input)
{
    Node::read(input);

    input.readObject("settings", settings);

    readDatabase(input.options);
}

void TileDatabase::write(vsg::Output& output) const
{
    Node::write(output);

    output.writeObject("settings", settings);
}

bool TileDatabase::readDatabase(vsg::ref_ptr<const vsg::Options> options)
{
    if (!settings || child) return false;

    if (settings->ellipsoidModel) setObject("EllipsoidModel", settings->ellipsoidModel);

    auto tileReader = tile::create(settings, options);

    auto local_options = options ? vsg::Options::create(*options) : vsg::Options::create();
    local_options->readerWriters.insert(local_options->readerWriters.begin(), tileReader);

    child = vsg::read_cast<vsg::Node>("root.tile", local_options);

    return child.valid();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  createBingMapsSettings
//
std::string_view vsg::find_field(const std::string& source, const std::string_view& start_match, const std::string_view& end_match)
{
    auto start_pos = source.find(start_match);
    if (start_pos == std::string::npos) return {};

    start_pos += start_match.size();

    auto end_pos = source.find(end_match, start_pos);
    if (end_pos == std::string::npos) return {};

    return {&source[start_pos], end_pos - start_pos};
}

void vsg::replace(std::string& source, const std::string_view& match, const std::string_view& replacement)
{
    for (;;)
    {
        auto pos = source.find(match);
        if (pos == std::string::npos) break;
        source.erase(pos, match.size());
        source.insert(pos, replacement);
    }
}

ref_ptr<TileDatabaseSettings> vsg::createBingMapsSettings(const std::string& imagerySet, const std::string& culture, const std::string& key, ref_ptr<const Options> options)
{
    auto settings = TileDatabaseSettings::create();

    // setup BingMaps settings
    settings->extents = {{-180.0, -90.0, 0.0}, {180.0, 90.0, 1.0}};
    settings->noX = 2;
    settings->noY = 2;
    settings->maxLevel = 19;
    settings->originTopLeft = true;
    settings->lighting = true;
    settings->projection = "EPSG:3857"; // Spherical Mecator

    if (!key.empty())
    {
        // read the meta data to set up the direct imageLayer URL.
        std::string metadata_url("https://dev.virtualearth.net/REST/V1/Imagery/Metadata/{imagerySet}?output=xml&key={key}");
        vsg::replace(metadata_url, "{imagerySet}", imagerySet);
        vsg::replace(metadata_url, "{key}", key);

        vsg::info("metadata_url = ", metadata_url);

        auto txt_options = vsg::Options::create(*options);
        txt_options->extensionHint = ".txt";

        if (auto metadata = vsg::read_cast<vsg::stringValue>(metadata_url, txt_options))
        {
            auto& str = metadata->value();
            vsg::info("metadata = ", str);

            std::string copyright(vsg::find_field(str, "<Copyright>", "</Copyright>"));
            std::string brandLogoUri(vsg::find_field(str, "<BrandLogoUri>", "</BrandLogoUri>"));
            std::string zoomMax(vsg::find_field(str, "<ZoomMax>", "</ZoomMax>"));

            vsg::info("copyright = ", copyright);
            vsg::info("brandLogoUri = ", brandLogoUri);
            vsg::info("zoomMax = ", zoomMax);

            settings->setValue("copyright", copyright);
            settings->setValue("logo", brandLogoUri);

            if (!brandLogoUri.empty())
            {
                auto logo = vsg::read_cast<vsg::Data>(brandLogoUri, options);
                vsg::info("logo = ", logo);
                // TODO need to create a logo subgraph
            }

            auto imageUrl = vsg::find_field(str, "<ImageUrl>", "</ImageUrl>");
            if (!imageUrl.empty())
            {
                std::string url(imageUrl);

                auto sumbdomain = vsg::find_field(str, "<ImageUrlSubdomains><string>", "</string>");
                if (!sumbdomain.empty())
                {
                    vsg::replace(url, "{subdomain}", sumbdomain);
                }

                vsg::replace(url, "{culture}", culture);

                settings->imageLayer = url;
            }
        }
    }

    if (settings->imageLayer.empty())
    {
        // direct access fallback
        settings->imageLayer = "https://ecn.t3.tiles.virtualearth.net/tiles/h{quadkey}.jpeg?g=1236";
    }

    return settings;
}

ref_ptr<TileDatabaseSettings> vsg::createOpenStreetMapSettings(ref_ptr<const Options> /*options*/)
{
    auto settings = vsg::TileDatabaseSettings::create();
    settings->extents = {{-180.0, -90.0, 0.0}, {180.0, 90.0, 1.0}};
    settings->noX = 1;
    settings->noY = 1;
    settings->maxLevel = 17;
    settings->originTopLeft = true;
    settings->lighting = false;
    settings->projection = "EPSG:3857"; // Spherical Mecator
    settings->imageLayer = "http://a.tile.openstreetmap.org/{z}/{x}/{y}.png";

    return settings;
}
