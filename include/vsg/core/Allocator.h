#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <map>
#include <memory>
#include <mutex>

namespace vsg
{
    enum AllocatorType : uint8_t
    {
        ALLOCATOR_TYPE_NO_DELETE = 0,
        ALLOCATOR_TYPE_NEW_DELETE,
        ALLOCATOR_TYPE_MALLOC_FREE,
        ALLOCATOR_TYPE_VSG_ALLOCATOR
    };

    enum AllocatorAffinity : uint32_t
    {
        ALLOCATOR_AFFINITY_OBJECTS,
        ALLOCATOR_AFFINITY_DATA,
        ALLOCATOR_AFFINITY_NODES,
        ALLOCATOR_AFFINITY_PHYSICS,
        ALLOCATOR_AFFINITY_LAST = ALLOCATOR_AFFINITY_PHYSICS + 1
    };

    /** extensible Allocator that handles allocation and deallocation of scene graph CPU memory,*/
    class VSG_DECLSPEC Allocator
    {
    public:
        explicit Allocator(size_t in_defaultAlignment = 8) :
            defaultAlignment(in_defaultAlignment) {}
        explicit Allocator(std::unique_ptr<Allocator> in_nestedAllocator, size_t in_defaultAlignment = 8) :
            defaultAlignment(in_defaultAlignment), nestedAllocator(std::move(in_nestedAllocator)) {}
        virtual ~Allocator() {}

        /// Allocator singleton
        static std::unique_ptr<Allocator>& instance();

        /// allocate from the pool of memory blocks, or allocate from a new memory block
        virtual void* allocate(std::size_t size, AllocatorAffinity allocatorAffinity = ALLOCATOR_AFFINITY_OBJECTS) = 0;

        /// deallocate, returning data to pool.
        virtual bool deallocate(void* ptr, std::size_t size) = 0;

        /// delete any MemoryBlock that are empty
        virtual size_t deleteEmptyMemoryBlocks() = 0;

        /// return the total available size of allocated MemoryBlocks
        virtual size_t totalAvailableSize() const = 0;

        /// return the total reserved size of allocated MemoryBlocks
        virtual size_t totalReservedSize() const = 0;

        /// return the total memory size of allocated MemoryBlocks
        virtual size_t totalMemorySize() const = 0;

        AllocatorType allocatorType = ALLOCATOR_TYPE_VSG_ALLOCATOR; // use MemoryBlocks by default

        virtual void setBlockSize(AllocatorAffinity allocatorAffinity, size_t blockSize) = 0;

        /// report stats about blocks of memory allocated.
        virtual void report(std::ostream& out) const = 0;

        mutable std::mutex mutex;
        size_t defaultAlignment = 8;

    protected:
        // if you are assigning a custom allocator you must retain the old allocator to manage the memory it allocated and needs to delete
        std::unique_ptr<Allocator> nestedAllocator;
    };

    /// allocate memory using vsg::Allocator::instance() if available, otherwise use std::malloc(size)
    extern VSG_DECLSPEC void* allocate(std::size_t size, AllocatorAffinity allocatorAffinity = ALLOCATOR_AFFINITY_OBJECTS);

    /// deallocate memory using vsg::Allocator::instance() if available, otherwise use std::free(ptr)
    extern VSG_DECLSPEC void deallocate(void* ptr, std::size_t size = 0);

    /// std container adapter for allocating with specific affinity
    template<typename T, vsg::AllocatorAffinity A>
    struct allocator_affinity_adapter
    {
        using value_type = T;

        allocator_affinity_adapter() = default;
        template<class U>
        explicit constexpr allocator_affinity_adapter(const allocator_affinity_adapter<U, A>&) noexcept {}

        template<class U>
        struct rebind
        {
            using other = allocator_affinity_adapter<U, A>;
        };

        value_type* allocate(std::size_t n)
        {
            return static_cast<value_type*>(vsg::allocate(n * sizeof(value_type), A));
        }

        void deallocate(value_type* ptr, std::size_t n)
        {
            vsg::deallocate(ptr, n * sizeof(value_type));
        }
    };

    template<class T, class U, vsg::AllocatorAffinity A>
    bool operator==(const allocator_affinity_adapter<T, A>&, const allocator_affinity_adapter<U, A>&) { return true; }

    template<class T, class U, vsg::AllocatorAffinity A>
    bool operator!=(const allocator_affinity_adapter<T, A>&, const allocator_affinity_adapter<U, A>&) { return false; }

    template<typename T>
    using allocator_affinity_objects = allocator_affinity_adapter<T, vsg::ALLOCATOR_AFFINITY_OBJECTS>;
    template<typename T>
    using allocator_affinity_data = allocator_affinity_adapter<T, vsg::ALLOCATOR_AFFINITY_DATA>;
    template<typename T>
    using allocator_affinity_nodes = allocator_affinity_adapter<T, vsg::ALLOCATOR_AFFINITY_NODES>;
    template<typename T>
    using allocator_affinity_physics = allocator_affinity_adapter<T, vsg::ALLOCATOR_AFFINITY_PHYSICS>;

} // namespace vsg
