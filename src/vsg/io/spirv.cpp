/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/io/spirv.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/utils/ShaderCompiler.h>

using namespace vsg;

template<typename T>
bool readFile(T& buffer, const vsg::Path& filename)
{
    std::ifstream fin(filename, std::ios::ate | std::ios::binary);
    if (!fin.is_open()) return false;

    size_t fileSize = fin.tellg();

    using value_type = typename T::value_type;
    size_t valueSize = sizeof(value_type);
    size_t bufferSize = (fileSize + valueSize - 1) / valueSize;
    buffer.resize(bufferSize);

    fin.seekg(0);
    fin.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    fin.close();

    // buffer.size() * valueSize

    return true;
}

spirv::spirv()
{
}

vsg::ref_ptr<vsg::Object> spirv::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    if (!compatibleExtension(filename, options, ".spv")) return {};

    vsg::Path found_filename = vsg::findFile(filename, options);
    if (!found_filename) return {};

    auto sm = vsg::ShaderModule::create();
    readFile(sm->code, found_filename);
    return sm;
}

ref_ptr<vsg::Object> spirv::read(std::istream& fin, ref_ptr<const Options> options) const
{
    if (!compatibleExtension(options, ".spv")) return {};

    fin.seekg (0, fin.end);
    size_t fileSize = fin.tellg();

    using value_type = vsg::ShaderModule::SPIRV::value_type;
    size_t valueSize = sizeof(value_type);
    size_t bufferSize = (fileSize + valueSize - 1) / valueSize;

    auto sm = vsg::ShaderModule::create();
    sm->code.resize(bufferSize);

    fin.seekg(0);
    fin.read(reinterpret_cast<char*>(sm->code.data()), fileSize);

    return sm;
}

ref_ptr<vsg::Object> spirv::read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options) const
{
    if (!compatibleExtension(options, ".spv")) return {};

    using value_type = vsg::ShaderModule::SPIRV::value_type;
    size_t valueSize = sizeof(value_type);
    size_t bufferSize = (size + valueSize - 1) / valueSize;

    auto sm = vsg::ShaderModule::create();
    sm->code.assign(reinterpret_cast<const value_type*>(ptr), reinterpret_cast<const value_type*>(ptr) + bufferSize);

    return sm;
}

bool spirv::write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    if (!compatibleExtension(filename, options, ".spv")) return false;

    const vsg::ShaderStage* ss = dynamic_cast<const vsg::ShaderStage*>(object);
    const vsg::ShaderModule* sm = ss ? ss->module.get() : dynamic_cast<const vsg::ShaderModule*>(object);
    if (sm)
    {
        if (sm->code.empty())
        {
            vsg::ShaderCompiler sc;
            if (!sc.compile(vsg::ref_ptr<vsg::ShaderStage>(const_cast<vsg::ShaderStage*>(ss))))
            {
                warn("spirv::write() Failed compile to spv.");
                return false;
            }
        }

        if (!sm->code.empty())
        {
            std::ofstream fout(filename);
            fout.write(reinterpret_cast<const char*>(sm->code.data()), sm->code.size() * sizeof(vsg::ShaderModule::SPIRV::value_type));
            fout.close();
            return true;
        }
    }
    return false;
}

bool spirv::getFeatures(Features& features) const
{
    features.extensionFeatureMap[".spv"] = static_cast<vsg::ReaderWriter::FeatureMask>(vsg::ReaderWriter::READ_FILENAME | vsg::ReaderWriter::WRITE_FILENAME);
    return true;
}
