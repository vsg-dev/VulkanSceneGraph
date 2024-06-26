#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

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
        explicit Allocator(size_t in_defaultAlignment = 4) :
            defaultAlignment(in_defaultAlignment) {}
        explicit Allocator(std::unique_ptr<Allocator> in_nestedAllocator, size_t in_defaultAlignment = 4) :
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
        size_t defaultAlignment = 4;

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // InstrusiveAllocator is the default Allocator implenentation
    //
    // Memory is allocated for fixed sized blocks, with indexing of allocated and available slots of memory
    // are stored within the same memory block that user memory allocation are made from.  The memory block
    // is created a contiguous block of 4 bytes Elements, where the Element is a union of bitfield linked list
    // market the beginning of the previous slot or the begging of the next, the status of whether the slot is
    // allocated or available, or an index when used as part of doubling linked list of free slots.
    //
    // The block allocation is done based on the type of object so all nodes, data or general objects are
    // allocated within the blocks containing objects of similar type.  This form of block allocation helps
    // scene graph traversal speeds by improving cache coherency/reducing cache missing as it ensures that
    // nodes etc. are packed in adjacent memory.
    //
    // The instrusive indexing means there is only a 4 byte panalty for each memory allocation, and a minimum
    // memory use per allocation of 12 bytes (3 Elements - 1 for the slot{previous, next, status} and 2 for the
    // previous and next free list indices.
    //
    // The maximum size of allocations within the block allocation is (2^15-2) * 4, allocations larger than this
    // are allocated using aligned versions of std::new and std::delete.
    //
    class VSG_DECLSPEC IntrusiveAllocator : public Allocator
    {
    public:
        explicit IntrusiveAllocator(size_t in_defaultAlignment = 4);
        explicit IntrusiveAllocator(std::unique_ptr<Allocator> in_nestedAllocator, size_t in_defaultAlignment = 4);

        ~IntrusiveAllocator();

        void report(std::ostream& out) const override;

        void* allocate(std::size_t size, AllocatorAffinity allocatorAffinity = ALLOCATOR_AFFINITY_OBJECTS) override;

        bool deallocate(void* ptr, std::size_t size) override;

        bool validate() const;

        size_t deleteEmptyMemoryBlocks() override;
        size_t totalAvailableSize() const override;
        size_t totalReservedSize() const override;
        size_t totalMemorySize() const override;
        void setBlockSize(AllocatorAffinity allocatorAffinity, size_t blockSize) override;

    protected:
        struct MemoryBlock
        {
            MemoryBlock(const std::string& in_name, size_t in_blockSize, size_t in_alignment);
            virtual ~MemoryBlock();

            std::string name;

            void* allocate(std::size_t size);
            bool deallocate(void* ptr, std::size_t size);

            void report(std::ostream& out) const;

            // bitfield packing of doubly-linked with status field into a 4 byte word
            struct Element
            {
                union
                {
                    uint32_t index;

                    struct
                    {
                        unsigned int previous : 15;
                        unsigned int next : 15;
                        unsigned int status : 2;
                    };
                };

                using Offset = decltype(previous);
                using Status = decltype(status);
                using Index = decltype(index);

                Element(size_t in_index) :
                    index(static_cast<Offset>(in_index)) {}

                Element(size_t in_previous, size_t in_next, unsigned int in_status) :
                    previous(static_cast<Offset>(in_previous)),
                    next(static_cast<Offset>(in_next)),
                    status(in_status) {}

                Element() = default;
                Element(const Element&) = default;
            };

            struct FreeList
            {
                Element::Index count = 0;
                Element::Index head = 0;
            };

            Element* memory = nullptr;
            Element* memoryEnd = nullptr;
            size_t capacity = 0;

            size_t alignment = 4; // min aligment is 4 { sizeof(Element) }
            size_t elementAlignment = 1;
            size_t blockAlignment = 16;
            size_t blockSize = 0;
            size_t maximumAllocationSize = 0;
            size_t firstSlot = 1;

            std::vector<FreeList> freeLists;

            bool validate() const;

            bool freeSlotsAvaible(size_t size) const;

            inline bool within(void* ptr) const { return memory <= ptr && ptr < memoryEnd; }

            size_t totalAvailableSize() const;
            size_t totalReservedSize() const;
            size_t totalMemorySize() const;

            struct SlotTester
            {
                SlotTester(Element* in_mem, size_t in_head) :
                    mem(in_mem), head(in_head){};

                const Element* mem = nullptr;
                size_t head = 0;

                struct Entry
                {
                    std::string name;
                    size_t position;
                    Element slot;
                    size_t previousFree;
                    size_t nextFree;
                };

                std::list<Entry> elements;

                void slot(size_t position, const std::string& name);

                void report(std::ostream& out);
            };

            static inline size_t computeMaxiumAllocationSize(size_t blockSize, size_t alignment)
            {
                return std::min(blockSize - alignment, size_t((1 << 15) - 2) * sizeof(Element));
            }
        };

        class MemoryBlocks
        {
        public:
            MemoryBlocks(IntrusiveAllocator* in_parent, const std::string& in_name, size_t in_blockSize, size_t in_alignment);
            virtual ~MemoryBlocks();

            IntrusiveAllocator* parent = nullptr;
            std::string name;
            size_t alignment = 4;
            size_t blockSize = 0;
            size_t maximumAllocationSize = 0;
            std::vector<std::shared_ptr<MemoryBlock>> memoryBlocks;
            std::shared_ptr<MemoryBlock> memoryBlockWithSpace;

            void* allocate(std::size_t size);
            void report(std::ostream& out) const;
            bool validate() const;

            size_t deleteEmptyMemoryBlocks();
            size_t totalAvailableSize() const;
            size_t totalReservedSize() const;
            size_t totalMemorySize() const;
        };

        std::vector<std::unique_ptr<MemoryBlocks>> allocatorMemoryBlocks;
        std::map<void*, std::shared_ptr<MemoryBlock>> memoryBlocks;
        std::map<void*, std::pair<size_t, size_t>> largeAllocations;
    };

} // namespace vsg
