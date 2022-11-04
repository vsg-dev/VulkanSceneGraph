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
