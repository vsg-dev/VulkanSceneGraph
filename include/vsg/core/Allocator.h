#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <list>
#include <map>
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

    class VSG_DECLSPEC MemorySlots
    {
    public:
        explicit MemorySlots(size_t availableMemorySize);

        using OptionalOffset = std::pair<bool, size_t>;
        OptionalOffset reserve(size_t size, size_t alignment);

        void release(size_t offset, size_t size);

        bool full() const { return _availableMemory.empty(); }

        size_t maximumAvailableSpace() const { return _availableMemory.empty() ? 0 : _availableMemory.rbegin()->first; }
        size_t totalAvailableSize() const;
        size_t totalReservedSize() const;
        size_t totalMemorySize() const { return _totalMemorySize; }

        void report() const;
        bool check() const;

    protected:
        using SizeOffsets = std::multimap<size_t, size_t>;
        using SizeOffset = SizeOffsets::value_type;
        SizeOffsets _availableMemory;

        using OffsetSizes = std::map<size_t, size_t>;
        using OffsetSize = OffsetSizes::value_type;
        OffsetSizes _offsetSizes;

        using OffsetAllocatedSlot = std::map<size_t, OffsetSize>;
        OffsetSizes _reservedOffsetSizes;

        size_t _totalMemorySize;
    };

    /** extensible Allocator that handles allocation and deallocation of scene graph CPU memory,*/
    class VSG_DECLSPEC Allocator
    {
    public:
        Allocator(std::unique_ptr<Allocator> in_nestedAllocator = {});

        virtual ~Allocator();

        /// Allocator singleton
        static std::unique_ptr<Allocator>& instance();

        virtual void* allocate(std::size_t size, AllocatorType allocatorType = ALLOCATOR_OBJECTS);
        virtual bool deallocate(void* ptr);

        void report() const;

    protected:
        struct MemoryBlock
        {
            MemoryBlock(size_t blockSize);
            virtual ~MemoryBlock();

            void* allocate(std::size_t size);
            bool deallocate(void* ptr);

            uint8_t* memory = nullptr;
            vsg::MemorySlots memorySlots;
        };

        struct MemoryBlocks
        {
            std::string name;
            size_t blockSize = 0;
            std::list<std::unique_ptr<MemoryBlock>> memoryBlocks;

            MemoryBlocks(const std::string& in_name, size_t in_blockSize);
            virtual ~MemoryBlocks();

            void* allocate(std::size_t size);
            bool deallocate(void* ptr);
        };

        // if you are assigning a custom allocator you mest retain the old allocator to manage the memory it allocated and needs to delete
        std::unique_ptr<Allocator> nestedAllocator;

        std::vector<std::unique_ptr<MemoryBlocks>> allocatorMemoryBlocks;
        mutable std::mutex mutex;
    };

    /// allocate memory using vsg::Allocator::instance() if avaiable, otherwise use std::malloc(size)
    void* allocate(std::size_t size, AllocatorType allocatorType = ALLOCATOR_OBJECTS);

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
            return static_cast<value_type*>(vsg::allocate(n * sizeof(value_type), vsg::ALLOCATOR_NODES));
        }

        void deallocate(value_type* ptr, std::size_t /*n*/)
        {
            vsg::deallocate(ptr);
        }
    };

    template<class T, class U>
    bool operator==(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return true; }

    template<class T, class U>
    bool operator!=(const allocator_affinity_nodes<T>&, const allocator_affinity_nodes<U>&) { return false; }

} // namespace vsg
