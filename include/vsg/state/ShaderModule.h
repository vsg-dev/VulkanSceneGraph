#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <fstream>

#include <vsg/vk/Device.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    // forward declare
    class Context;

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

    class VSG_DECLSPEC ShaderModule : public Inherit<Object, ShaderModule>
    {
    public:
        using SPIRV = std::vector<uint32_t>;

        ShaderModule();
        ShaderModule(const std::string& in_source);
        ShaderModule(const SPIRV& in_code);
        ShaderModule(const std::string& source, const SPIRV& in_code);

        /// VkShaderModuleCreateInfo settings
        std::string source;
        SPIRV code;

        /// Vulkan VkShaderModule handle
        VkShaderModule vk(uint32_t deviceID) const { return _implementation[deviceID]->_shaderModule; }

        void read(Input& input) override;
        void write(Output& output) const override;

        static ref_ptr<ShaderModule> read(const std::string& filename);

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }


    protected:
        virtual ~ShaderModule();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, ShaderModule* shader);

            virtual ~Implementation();

            VkShaderModule _shaderModule;
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::ShaderModule);

} // namespace vsg
