#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>

#include <list>
#include <vector>

namespace vsg
{
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
        explicit IntrusiveAllocator(size_t in_defaultAlignment = 8);
        explicit IntrusiveAllocator(std::unique_ptr<Allocator> in_nestedAllocator, size_t in_defaultAlignment = 8);

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
        struct VSG_DECLSPEC MemoryBlock
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

                explicit Element(Index in_index) :
                    index(static_cast<Offset>(in_index)) {}

                Element(Offset in_previous, Offset in_next, Status in_status) :
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

            size_t alignment = 8; // min aligment is 4 { sizeof(Element) }
            size_t blockAlignment = 16;
            size_t blockSize = 0;
            size_t maximumAllocationSize = 0;
            Element::Index elementAlignment = 1;
            Element::Index firstSlot = 1;
            Element::Index capacity = 0;

            std::vector<FreeList> freeLists;

            bool validate() const;

            bool freeSlotsAvaible(size_t size) const;

            inline bool within(const void* ptr) const { return memory <= ptr && ptr < memoryEnd; }

            size_t totalAvailableSize() const;
            size_t totalReservedSize() const;
            size_t totalMemorySize() const;

            // used for debugging only.
            struct VSG_DECLSPEC SlotTester
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

        class VSG_DECLSPEC MemoryBlocks
        {
        public:
            MemoryBlocks(IntrusiveAllocator* in_parent, const std::string& in_name, size_t in_blockSize, size_t in_alignment);
            virtual ~MemoryBlocks();

            IntrusiveAllocator* parent = nullptr;
            std::string name;
            size_t alignment = 8;
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
