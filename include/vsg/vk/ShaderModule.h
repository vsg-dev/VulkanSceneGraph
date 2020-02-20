#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <fstream>

#include <vsg/vk/Device.h>
#include <vsg/vk/implementation_buffer.h>

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
        using Source = std::string;
        using SPIRV = std::vector<uint32_t>;

        ShaderModule();
        ShaderModule(const Source& spirv);
        ShaderModule(const SPIRV& spirv);
        ShaderModule(const Source& source, const SPIRV& spirv);

        void read(Input& input) override;
        void write(Output& output) const override;

        std::string& source() { return _source; }
        const std::string& source() const { return _source; }

        SPIRV& spirv() { return _spirv; }
        const SPIRV& spirv() const { return _spirv; }

        using Result = vsg::Result<ShaderModule, VkResult, VK_SUCCESS>;
        static Result read(const std::string& filename);

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = nullptr; }
        void release() { for(auto& imp : _implementation) imp = nullptr; }

        VkShaderModule vk(uint32_t deviceID) const { return _implementation.vk(deviceID); }

    protected:
        virtual ~ShaderModule();

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkShaderModule shaderModule, Device* device, AllocationCallbacks* allocator);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Create a ComputePipeline.*/
            static Result create(Device* device, ShaderModule* shader, AllocationCallbacks* allocator = nullptr);

            VkShaderModule vk() const { return _shaderModule; }

            VkShaderModule _shaderModule;
            ref_ptr<Device> _device;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        implementation_buffer<Implementation> _implementation;

        std::string _source;
        SPIRV _spirv;
    };
    VSG_type_name(vsg::ShaderModule);

} // namespace vsg
