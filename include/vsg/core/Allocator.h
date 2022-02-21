#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

#include <mutex>
#include <map>
#include <memory>

namespace vsg
{

    enum MemoryAffinity
    {
        MEMORY_AFFINITY_OBJECTS,
        MEMORY_AFFINITY_DATA,
        MEMORY_AFFINITY_NODES
    };

    /** extensible Allocator that handles allocation and deallocation of scene graph CPU memory,*/
    class VSG_DECLSPEC Allocator
    {
    public:
        Allocator();
        virtual ~Allocator();

        /// Allocator singleton
        static std::unique_ptr<Allocator>& instance();

        virtual void* allocate(std::size_t count, MemoryAffinity memoryAffinity = MEMORY_AFFINITY_OBJECTS);
        virtual void deallocate(void* ptr);

    protected:

    };

    /// allocate memory using vsg::Allocator::instance() if avaiable, otherwise use std::malloc(count)
    void* allocate(std::size_t count, MemoryAffinity memoryAffinity = MEMORY_AFFINITY_OBJECTS);

    /// deallocate memory using vsg::Allocator::instance() if avaiable, otherwise use std::free(ptr)
    void deallocate(void* ptr);


    /// std container adapter for allocating with MEMORY_AFFINITY_NODES
    template<typename T>
    struct allocator_affinity_nodes
    {
        using value_type = T;

        allocator_affinity_nodes() = default;
        template<class U>
        constexpr allocator_affinity_nodes(const allocator_affinity_nodes<U>&) noexcept {}

        value_type* allocate(std::size_t n)
        {
            return static_cast<value_type*>(vsg::allocate(n * sizeof(value_type), vsg::MEMORY_AFFINITY_NODES));
        }

        void deallocate(value_type* ptr, std::size_t /*n*/)
        {
            return vsg::deallocate(ptr);
        }
    };

    template<class T, class U>
    bool operator==(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return true; }

    template<class T, class U>
    bool operator!=(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return false; }


} // namespace vsg
