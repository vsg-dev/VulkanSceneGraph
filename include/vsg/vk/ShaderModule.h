#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>
#include <fstream>

namespace vsg
{
    template<typename T>
    bool readFile(T& buffer, const std::string& filename)
    {
        std::ifstream fin(filename, std::ios::ate | std::ios::binary);
        if (!fin.is_open()) return false;

        size_t fileSize = fin.tellg();

        using value_type = typename T::value_type;
        size_t valueSize = sizeof(value_type);
        buffer.resize(fileSize/valueSize);

        fin.seekg(0);
        fin.read(buffer.data(), buffer.size()*valueSize);
        fin.close();

        return true;
    }

    class VSG_EXPORT Shader : public Object
    {
    public:
        using Contents = std::vector<char>;

        Shader(VkShaderStageFlagBits stage, const std::string& entryPointName, const Contents& contents);

        VkShaderStageFlagBits stage() const { return _stage; }
        const std::string& entryPointName() const { return _entryPointName; }
        const Contents& contents() const { return _contents; }

        using Result = vsg::Result<Shader, VkResult, VK_SUCCESS>;
        static Result read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename);

    protected:
        virtual ~Shader();

        VkShaderStageFlagBits   _stage;
        std::string             _entryPointName;
        Contents                _contents;
    };

    class VSG_EXPORT ShaderModule : public vsg::Object
    {
    public:
        ShaderModule(VkShaderModule shaderModule, Device* device, Shader* shader, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<ShaderModule, VkResult, VK_SUCCESS>;

        static Result create(Device* device, Shader* shader, AllocationCallbacks* allocator=nullptr);

        operator VkShaderModule () const { return _shaderModule; }

        const Shader* getShader() const { return _shader; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~ShaderModule();

        VkShaderModule                  _shaderModule;

        ref_ptr<Device>                 _device;
        ref_ptr<Shader>                 _shader;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    using ShaderModules = std::vector<ref_ptr<ShaderModule>>;

}
