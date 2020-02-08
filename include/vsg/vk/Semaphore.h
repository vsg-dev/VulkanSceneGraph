#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

namespace vsg
{
    class VSG_DECLSPEC Semaphore : public Inherit<Object, Semaphore>
    {
    public:
        Semaphore(VkSemaphore Semaphore, Device* device, AllocationCallbacks* allocator = nullptr);

        using Result = vsg::Result<Semaphore, VkResult, VK_SUCCESS>;
        static Result create(Device* device, void* pNextCreateInfo = nullptr, AllocationCallbacks* allocator = nullptr);

        operator VkSemaphore() const { return _semaphore; }

        VkPipelineStageFlags& pipelineStageFlags() { return _pipelineStageFlags; }
        const VkPipelineStageFlags& pipelineStageFlags() const { return _pipelineStageFlags; }

        std::atomic_uint& numDependentSubmissions() { return _numDependentSubmissions; }

        const VkSemaphore* data() const { return &_semaphore; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Semaphore();

        VkSemaphore _semaphore;
        VkPipelineStageFlags _pipelineStageFlags = 0;
        std::atomic_uint _numDependentSubmissions = 0;
        ref_ptr<Device> _device;
        ref_ptr<AllocationCallbacks> _allocator;
    };

    using Semaphores = std::vector<ref_ptr<Semaphore>>;

} // namespace vsg
