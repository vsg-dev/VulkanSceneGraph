/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/spirv.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/vk/ShaderCompiler.h>

#include <iostream>

using namespace vsg;

template<typename T>
bool readFile(T& buffer, const std::string& filename)
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
    auto ext = vsg::lowerCaseFileExtension(filename);
    if (ext == ".spv")
    {
        vsg::Path found_filename = vsg::findFile(filename, options);
        if (found_filename.empty()) return {};

        vsg::ShaderModule::SPIRV spirv_module;
        readFile(spirv_module, found_filename);

        auto sm = vsg::ShaderModule::create(spirv_module);
        return sm;
    }
    return vsg::ref_ptr<vsg::Object>();
}

bool spirv::write(const vsg::Object* object, const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> /*options*/) const
{
    auto ext = vsg::lowerCaseFileExtension(filename);
    if (ext == ".spv")
    {
        const vsg::ShaderStage* ss = dynamic_cast<const vsg::ShaderStage*>(object);
        const vsg::ShaderModule* sm = ss ? ss->module.get() : dynamic_cast<const vsg::ShaderModule*>(object);
        if (sm)
        {
            if (sm->code.empty())
            {
                vsg::ShaderCompiler sc;
                if (!sc.compile(vsg::ref_ptr<vsg::ShaderStage>(const_cast<vsg::ShaderStage*>(ss))))
                {
                    std::cout << "spirv::write() Failed compile to spv." << std::endl;
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
    }
    return false;
}

bool spirv::getFeatures(Features& features) const
{
    features.extensionFeatureMap[".spv"] = static_cast<vsg::ReaderWriter::FeatureMask>(vsg::ReaderWriter::READ_FILENAME | vsg::ReaderWriter::WRITE_FILENAME);
    return true;
}
