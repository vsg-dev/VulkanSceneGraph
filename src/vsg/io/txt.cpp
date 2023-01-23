/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/io/txt.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/utils/ShaderCompiler.h>

using namespace vsg;


static std::set<vsg::Path> s_txt_extensionSupported{
        ".txt",
        ".text",
        ".md",
        ".json",
        ".xml",
        ".sh",
        ".bat"
    };

bool txt::extensionSupported(const vsg::Path& path)
{
    return s_txt_extensionSupported.count(path);
}

txt::txt()
{
    supportedExtensions = s_txt_extensionSupported;
}


vsg::ref_ptr<vsg::Object> txt::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    if (!compatibleExtension(filename, options, supportedExtensions)) return {};

    vsg::Path found_filename = vsg::findFile(filename, options);
    if (!found_filename) return {};

    std::ifstream fin(filename, std::ios::ate | std::ios::binary);
    if (!fin.is_open()) return {};

    return read(fin, options);
}

ref_ptr<vsg::Object> txt::read(std::istream& fin, ref_ptr<const Options> options) const
{
    if (!compatibleExtension(options, supportedExtensions)) return {};

    vsg::ref_ptr<vsg::stringValue> contents = vsg::stringValue::create();
    auto& buffer = contents->value();

    fin.seekg (0, fin.end);
    size_t fileSize = fin.tellg();
    buffer.resize(fileSize);

    fin.seekg(0);
    fin.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    return contents;
}

ref_ptr<vsg::Object> txt::read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options) const
{
    if (!compatibleExtension(options, supportedExtensions)) return {};

    return vsg::stringValue::create(std::string(reinterpret_cast<const char*>(ptr), size));
}

bool txt::getFeatures(Features& features) const
{
    for(auto& ext : supportedExtensions)
    {
        features.extensionFeatureMap[ext] = static_cast<FeatureMask>(READ_FILENAME | READ_ISTREAM | READ_MEMORY);
    }
    return true;
}
