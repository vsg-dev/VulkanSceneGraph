#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/MemorySlots.h>

#include <memory>
#include <mutex>
#include <vector>

namespace vsg
{
    enum AllocatorType : uint8_t
    {
        ALLOCATOR_NO_DELETE = 0,
        ALLOCATOR_NEW_DELETE,
        ALLOCATOR_MALLOC_FREE,
        ALLOCATOR_OBJECTS,
        ALLOCATOR_DATA,
        ALLOCATOR_NODES,
        ALLOCATOR_LAST = ALLOCATOR_NODES + 1
    };

    /** extensible Allocator that handles allocation and deallocation of scene graph CPU memory,*/
    class VSG_DECLSPEC Allocator
    {
    public:
        Allocator(std::unique_ptr<Allocator> in_nestedAllocator = {});

        virtual ~Allocator();

        /// Allocator singleton
        static std::unique_ptr<Allocator>& instance();

        /// allocate from the pool of memory blocks, or allocate from a new memory bock
        virtual void* allocate(std::size_t size, AllocatorType allocatorType = ALLOCATOR_OBJECTS);

        /// deallocate returning data to pool.
        virtual bool deallocate(void* ptr, std::size_t size);

        /// report stats about block of memory allocated.
        virtual void report(std::ostream& out) const;

        int memoryTracking = MEMORY_TRACKING_DEFAULT;

        /// set the MemoryTracking member of of the vsg::Allocator and all the MemoryBlocks that it manages.
        void setMemoryTracking(int mt);

    protected:
        struct MemoryBlock
        {
            MemoryBlock(size_t blockSize, int memoryTracking);
            virtual ~MemoryBlock();

            void* allocate(std::size_t size);
            bool deallocate(void* ptr, std::size_t size);

            uint8_t* memory = nullptr;
            vsg::MemorySlots memorySlots;
        };

        struct MemoryBlocks
        {
            Allocator* parent = nullptr;
            std::string name;
            size_t blockSize = 0;
            std::list<std::unique_ptr<MemoryBlock>> memoryBlocks;

            MemoryBlocks(Allocator* in_parent, const std::string& in_name, size_t in_blockSize);
            virtual ~MemoryBlocks();

            void* allocate(std::size_t size);
            bool deallocate(void* ptr, std::size_t size);
        };

        // if you are assigning a custom allocator you mest retain the old allocator to manage the memory it allocated and needs to delete
        std::unique_ptr<Allocator> nestedAllocator;

        std::vector<std::unique_ptr<MemoryBlocks>> allocatorMemoryBlocks;
        mutable std::mutex mutex;
    };

    /// allocate memory using vsg::Allocator::instance() if avaiable, otherwise use std::malloc(size)
    void* allocate(std::size_t size, AllocatorType allocatorType = ALLOCATOR_OBJECTS);

    /// deallocate memory using vsg::Allocator::instance() if avaiable, otherwise use std::free(ptr)
    void deallocate(void* ptr, std::size_t size = 0);

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
            return static_cast<value_type*>(vsg::allocate(n * sizeof(value_type), vsg::ALLOCATOR_NODES));
        }

        void deallocate(value_type* ptr, std::size_t n)
        {
            vsg::deallocate(ptr, n * sizeof(value_type));
        }
    };

    template<class T, class U>
    bool operator==(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return true; }

    template<class T, class U>
    bool operator!=(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return false; }

} // namespace vsg
