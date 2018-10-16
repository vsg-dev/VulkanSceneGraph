#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/vk/AllocationCallbacks.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace vsg
{
    using Names = std::vector<const char*>;

    extern VSG_DECLSPEC Names validateInstancelayerNames(const Names& names);

    class VSG_DECLSPEC Instance : public Inherit<Object, Instance>
    {
    public:
        Instance(VkInstance instance, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Instance, VkResult, VK_SUCCESS>;
        static Result create(Names& instanceExtensions, Names& layers, AllocationCallbacks* allocator=nullptr);

        operator VkInstance() const { return _instance; }
        VkInstance getInstance() const { return _instance; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

    protected:

        virtual ~Instance();

        VkInstance                      _instance;
        ref_ptr<AllocationCallbacks>    _allocator;
    };
}
