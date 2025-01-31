/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/IntrusiveAllocator.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Logger.h>

#include <algorithm>
#include <cstddef>
#include <iostream>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::InstrusiveAllocator
//

#define DEBUG_ALLOCATOR 0

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryBlock
//
IntrusiveAllocator::MemoryBlock::MemoryBlock(const std::string& in_name, size_t in_blockSize, size_t in_alignment) :
    name(in_name),
    alignment(in_alignment),
    blockSize(in_blockSize)
{
    alignment = std::max(alignment, sizeof(Element)); // we need to be a multiple of sizeof(value_type)
    elementAlignment = static_cast<Element::Index>(alignment / sizeof(Element));

    blockAlignment = std::max(alignment, alignof(std::max_align_t));
    blockAlignment = std::max(blockAlignment, size_t{16});

    // round blockSize up to nearest aligned size
    blockSize = ((blockSize + alignment - 1) / alignment) * alignment;

    memory = static_cast<Element*>(operator new(blockSize, std::align_val_t{blockAlignment}));
    memoryEnd = memory + blockSize / sizeof(Element);
    capacity = static_cast<Element::Index>(blockSize / alignment);
    firstSlot = static_cast<Element::Index>(((1 + elementAlignment) / elementAlignment) * elementAlignment - 1);

    Element::Index max_slot_size = (1 << 15);

    // // vsg::debug("    capacity = ", capacity, ", max_slot_size = ", max_slot_size);

    // set up the free tracking to encompass the whole buffer
    // start at element before the first aligned element so that position 0 can be used to mark beginning or end of free lists
    freeLists.emplace_back();
    FreeList& freeList = freeLists.front();
    freeList.count = 0;
    freeList.head = firstSlot;
    maximumAllocationSize = computeMaxiumAllocationSize(blockSize, alignment);

    // mark the first element as 0.
    memory[0].index = 0;

    Element::Index previous_position = 0; // 0 marks the beginning of the free list
    Element::Index position = firstSlot;
    for (; position < capacity;)
    {
        Element::Index aligned_start = ((position + max_slot_size) / elementAlignment) * elementAlignment;
        Element::Index next_position = std::min(aligned_start - 1, capacity);

        memory[position] = Element{(previous_position == 0) ? 0 : (position - previous_position), next_position - position, 1};
        memory[position + 1].index = static_cast<Element::Index>(previous_position);
        memory[position + 2].index = static_cast<Element::Index>((next_position < capacity) ? next_position : 0);
        previous_position = position;
        position = next_position;
        ++freeList.count;
    }

#if DEBUG_ALLOCATOR
    std::cout << "IntrusiveAllocator::MemoryBlock::MemoryBlock(" << in_blockSize << ", " << in_alignment << ")" << std::endl;

    std::cout << "blockSize = " << blockSize << std::endl;
    std::cout << "capacity = " << capacity << std::endl;
    std::cout << "totalReservedSize = " << totalReservedSize() << std::endl;
    std::cout << "totalAvailableSize = " << totalAvailableSize() << std::endl;
    std::cout << "alignment = " << alignment << std::endl;
    std::cout << "elementAlignment = " << elementAlignment << std::endl;
    std::cout << "freeList.head = " << freeList.head << std::endl;

    report(std::cout);
#endif
}

IntrusiveAllocator::MemoryBlock::~MemoryBlock()
{
    operator delete(memory, std::align_val_t{blockAlignment});
}

bool IntrusiveAllocator::MemoryBlock::freeSlotsAvaible(size_t size) const
{
    if (size > maximumAllocationSize) return false;

    for (const auto& freeList : freeLists)
    {
        if (freeList.count > 0) return true;
    }
    return false;
}

void* IntrusiveAllocator::MemoryBlock::allocate(std::size_t size)
{
#if DEBUG_ALLOCATOR
    if (!validate()) std::cout << "ERROR detected before IntrusiveAllocator::MemoryBlock::allocate(" << size << ") " << this << std::endl;
#endif

    // check if maximumAllocationSize is big enough
    if (size > maximumAllocationSize) return nullptr;

    const size_t minimumNumElementsInSlot = 3;

    for (auto& freeList : freeLists)
    {
        // check if freeList has available slots
        if (freeList.count == 0) continue;

        Element::Index freePosition = freeList.head;
        while (freePosition != 0)
        {
            auto& slot = memory[freePosition];
            if (slot.status != 1)
            {
                throw "Warning: allocated slot found in freeList";
            }

            Element::Index previousFreePosition = memory[freePosition + 1].index;
            Element::Index nextFreePosition = memory[freePosition + 2].index;

            size_t slotSpace = static_cast<size_t>(slot.next);
            if (slot.next == 0)
            {
                std::cerr << "Warn: IntrusiveAllocator::MemoryBlock::allocate(" << size << ") slot = { " << static_cast<uint16_t>(slot.previous) << ", " << static_cast<uint16_t>(slot.next) << ", " << static_cast<uint16_t>(slot.status) << " }" << std::endl;
            }

            Element::Index nextPosition = freePosition + static_cast<Element::Index>(slotSpace);
            size_t slotSize = sizeof(Element) * (slotSpace - 1);

            if (size <= slotSize)
            {
                // we can us slot for memory;

                size_t numElementsToBeUsed = std::max((size + sizeof(Element) - 1) / sizeof(Element), minimumNumElementsInSlot);
                Element::Index nextAlignedStart = static_cast<Element::Index>(((freePosition + 1 + numElementsToBeUsed + elementAlignment) / elementAlignment) * elementAlignment);
                Element::Index minimumAlignedEnd = nextAlignedStart + static_cast<Element::Index>(minimumNumElementsInSlot);
#if DEBUG_ALLOCATOR
                std::cout << "allocating, size = " << size << ", numElementsToBeUsed = " << numElementsToBeUsed << ", freePosition = " << freePosition << ", nextPosition = " << nextPosition << ", nextAlignedStart = " << nextAlignedStart << ", minimumAlignedEnd = " << minimumAlignedEnd << std::endl;
#endif
                if (minimumAlignedEnd < nextPosition)
                {

                    // enough space in slot to split, so adjust
                    Element::Index newSlotPosition = nextAlignedStart - 1;
                    slot.next = static_cast<Element::Offset>(newSlotPosition - freePosition);

#if DEBUG_ALLOCATOR
                    std::cout << "splitting slot newSlotPosition = " << newSlotPosition << std::endl;
#endif
                    // set up the new slot as a free slot
                    auto& newSlot = memory[newSlotPosition] = Element(slot.next, static_cast<Element::Offset>(nextPosition - newSlotPosition), 1);
                    memory[newSlotPosition + 1].index = previousFreePosition;
                    memory[newSlotPosition + 2].index = nextFreePosition;

                    if (previousFreePosition != 0)
                    {
                        // need to update the previous slot in the free list
                        memory[previousFreePosition + 2].index = newSlotPosition; // set previous free slots next index to the newly created slot
                    }

                    if (nextFreePosition != 0)
                    {
                        // need to update the previous slot in the free list
                        memory[nextFreePosition + 1].index = newSlotPosition; // set next free slots previous index to the newly created slot
                    }

                    if (nextPosition < capacity)
                    {
                        auto& nextSlot = memory[nextPosition];
                        nextSlot.previous = newSlot.next;
                    }

                    if (freePosition == freeList.head)
                    {
                        // slot was at head of freeList so move it to the new slot position
                        freeList.head = newSlotPosition;
                    }
                }
                else
                {

                    // std::cout<<"Removing slot as it's fully used freePosition = "<<freePosition<<", previousFreePosition = "<<previousFreePosition<<", nextFreePosition = "<<nextFreePosition<<std::endl;

                    // not enough space to split up slot, so remove it from freeList
                    if (previousFreePosition != 0)
                    {
                        // need to update the previous slot in the free list
                        memory[previousFreePosition + 2].index = nextFreePosition;
                    }

                    if (nextFreePosition != 0)
                    {
                        // need to update the previous slot in the free list
                        memory[nextFreePosition + 1].index = previousFreePosition;
                    }

                    if (freePosition == freeList.head)
                    {
                        // slot was at head of freeList so move it to the new slot position
                        freeList.head = nextFreePosition;
                    }

                    // one list free slot availalbe
                    --freeList.count;
                }

                slot.status = 0; // mark slot as allocated

#if DEBUG_ALLOCATOR
                if (validate())
                    std::cout << "IntrusiveAllocator::MemoryBlock::allocate(" << size << ") " << this << " allocated = " << &memory[freePosition + 1] << " freePosition = " << freePosition << ", slot = {" << slot.previous << ", " << slot.next << ", " << static_cast<uint32_t>(slot.status) << std::endl;
                else
                    std::cout << "ERROR detected after IntrusiveAllocator::MemoryBlock::allocate(" << size << ") " << this << " allocated = " << &memory[freePosition + 1] << std::endl;
#endif

                return &memory[freePosition + 1];
            }

            freePosition = nextFreePosition;
        }
    }

#if DEBUG_ALLOCATOR
    std::cout << "IntrusiveAllocator::MemoryBlock::allocator(" << size << ") " << this << " No space found" << std::endl;
#endif

    return nullptr;
}

void IntrusiveAllocator::MemoryBlock::SlotTester::slot(size_t position, const std::string& name)
{
    if (mem[position].status == 0)
        elements.push_back(Entry{name, position, mem[position], 0, 0});
    else
        elements.push_back(Entry{name, position, mem[position], mem[position + 1].index, mem[position + 2].index});
}

void IntrusiveAllocator::MemoryBlock::SlotTester::report(std::ostream& out)
{
    out << "head = " << head << std::endl;
    for (const auto& entry : elements)
    {
        out << "    " << entry.name << ", pos = " << entry.position << " slot { " << entry.slot.previous << ", " << entry.slot.next << ", " << static_cast<uint16_t>(entry.slot.status) << " } ";
        if (entry.slot.status != 0)
            out << " previous free = " << entry.previousFree << ",  next free = " << entry.nextFree << std::endl;
        else
            out << std::endl;
    }
}

bool IntrusiveAllocator::MemoryBlock::deallocate(void* ptr, std::size_t /*size*/)
{
    if (within(ptr))
    {
        auto& freeList = freeLists.front();
        size_t maxSize = 1 + maximumAllocationSize / sizeof(Element);

        //
        // sequential slots around the slot to be deallocated are named:
        //    PP (Previous' Previous), P (Previous), C (Current slot being deallocated), N (Next), NN (Next's Next)
        //
        // the FreeList linked list entries of interest are named:
        //    PPF (Previous' Previous Free), PNF (Previous's Next Free), NPF (Next's Previous Free), NNF (Next's Next Free)
        //

        Element::Index C = static_cast<Element::Index>(static_cast<Element*>(ptr) - memory) - 1;
        auto& slot = memory[C];

#if DEBUG_ALLOCATOR
        if (validate())
        {
            std::cout << "IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << " C = " << C << ", slot = {" << slot.previous << ", " << slot.next << ", " << static_cast<uint32_t>(slot.status) << std::endl;
        }
        else
        {
            std::cout << "ERROR detected befpre IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << std::endl;
        }
#endif

        if (slot.next == 0)
        {
            std::cerr << "Warn: IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ") C = " << C << ", slot = { " << slot.previous << ", " << slot.next << ", " << slot.status << " }" << std::endl;
            throw "slot.ext == 0";
        }

        if (slot.status != 0)
        {
            std::cerr << "Warn: IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ") C = " << C << ", Attempt to deallocate already available slot : slot = { " << slot.previous << ", " << slot.next << ", " << slot.status << " }" << std::endl;
            throw "Attempt to deallocate already available slot";
        }

        // set up the indices for the previous and next slots
        Element::Index P = (slot.previous > 0) ? (C - static_cast<Element::Index>(slot.previous)) : 0;
        Element::Index N = C + static_cast<Element::Index>(slot.next);
        if (N >= capacity) N = 0;

        // set up the indices for the previous free entry
        Element::Index PPF = 0;
        Element::Index PNF = 0;
        if (P != 0)
        {
            if (memory[P].status != 0)
            {
                PPF = memory[P + 1].index;
                PNF = memory[P + 2].index;
            }
        }

        // set up the indices for the next free entries
        Element::Index NN = 0;
        Element::Index NPF = 0;
        Element::Index NNF = 0;
        if (N != 0)
        {
            NN = N + static_cast<Element::Index>(memory[N].next);
            if (NN >= capacity) NN = 0;

            if (memory[N].status != 0)
            {
                NPF = memory[N + 1].index;
                NNF = memory[N + 2].index;
            }
        }

        // 3 way merge of P, C and C
        auto mergePCN = [&]() -> void {
#if DEBUG_ALLOCATOR
            SlotTester before(memory, freeList.head);
            before.slot(P, "P");
            before.slot(C, "C");
            before.slot(N, "N");
            before.slot(PPF, "PPF");
            before.slot(PNF, "PNF");
            before.slot(NPF, "NPF");
            before.slot(NNF, "NNF");
#endif
            // update slots for the merge
            memory[P].next += memory[C].next + memory[N].next;
            if (NN != 0) memory[NN].previous = memory[P].next;

            // update freeList linked list entries
            if (PNF == N) // also implies NPF == P
            {
                // case 1. in order sequential
#if DEBUG_ALLOCATOR
                std::cout << "       case 1. in order sequential" << std::endl;
#endif
                memory[P + 2].index = NNF;
                if (NNF != 0) memory[NNF + 1].index = P;
            }
            else if (PPF == N) // also implies NNF == P
            {
                // case 2. reverse sequential
#if DEBUG_ALLOCATOR
                std::cout << "       case 2. reverse sequential" << std::endl;
#endif
                if (freeList.head == N)
                {
                    freeList.head = P;
                    memory[P + 1].index = 0;
                }
                else
                {
                    memory[P + 1].index = NPF;
                    if (NPF != 0) memory[NPF + 2].index = P;
                }
            }
            else // P and N aren't directly connected within the freeList
            {
                // case 3. disconnected
#if DEBUG_ALLOCATOR
                std::cout << "       case 3. disconnected" << std::endl;
#endif
                if (NPF != 0) memory[NPF + 2].index = NNF;
                if (NNF != 0) memory[NNF + 1].index = NPF;

                if (freeList.head == N)
                {
                    freeList.head = NNF;
                }
            }

            // N slot is nolonger a seperate free slot so decrement free count
            --freeList.count;

#if DEBUG_ALLOCATOR
            if (!validate())
            {
                SlotTester after(memory, freeList.head);
                after.slot(P, "P");
                after.slot(PPF, "PPF");
                after.slot(PNF, "PNF");
                after.slot(NPF, "NPF");
                after.slot(NNF, "NNF");

                std::cout << "ERROR detected after mergePCN() IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << std::endl;

                std::cout << "Before: ";
                before.report(std::cout);
                std::cout << "After: ";
                after.report(std::cout);
            }
#endif
        };

        // 2 way merge of P and C
        auto mergePC = [&]() -> void {
            // update slots for the merge
            memory[P].next += memory[C].next;
            if (N != 0) memory[N].previous = memory[P].next;

                // freeList linked list entries will not need updating.

#if DEBUG_ALLOCATOR
            if (!validate())
            {
                std::cout << "ERROR detected after mergePC() IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << std::endl;
            }
#endif
        };

        // 2 way merge of C and N
        auto mergeCN = [&]() -> void {
            // update slots for merge
            memory[C].status = 1;
            memory[C].next += memory[N].next;
            if (NN != 0) memory[NN].previous = memory[C].next;

            // update freeList linked list entries
            if (NPF != 0) memory[NPF + 2].index = C;
            if (NNF != 0) memory[NNF + 1].index = C;
            memory[C + 1].index = NPF;
            memory[C + 2].index = NNF;

            // if N was the head then change head to C
            if (freeList.head == N) freeList.head = C;

#if DEBUG_ALLOCATOR
            if (!validate())
            {
                std::cout << "ERROR detected after mergeCN() IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << std::endl;
            }
#endif
        };

        // standalone insertion of C into head of freeList
        auto standalone = [&]() -> void {
            memory[C].status = 1;
            memory[C + 1].index = 0;
            memory[C + 2].index = freeList.head;

            if (freeList.head != 0)
            {
                memory[freeList.head + 1].index = C; // set previous heads previousFree to C.
            }

            // set the head to C.
            freeList.head = C;

            // Inserted new free slot so increment free count
            ++freeList.count;

#if DEBUG_ALLOCATOR
            if (!validate())
            {
                std::cout << "ERROR detected after standalone() IntrusiveAllocator::MemoryBlock::deallocate(" << ptr << ", " << size << ") " << this << " C = " << C << ", memory[C + 2].index = " << memory[C + 2].index << std::endl;
            }
#endif
        };

        if (P != 0 && memory[P].status != 0)
        {
            if (N != 0 && memory[N].status != 0)
            {
                if ((static_cast<size_t>(memory[P].next) + static_cast<size_t>(memory[C].next) + static_cast<size_t>(memory[N].next)) <= maxSize)
                    mergePCN();
                else if ((static_cast<size_t>(memory[P].next) + static_cast<size_t>(memory[C].next)) <= maxSize)
                    mergePC(); // merge P and C
                else if ((static_cast<size_t>(memory[C].next) + static_cast<size_t>(memory[N].next)) <= maxSize)
                    mergeCN(); // merge C and N
                else
                    standalone(); // C is standalone
            }
            else if ((static_cast<size_t>(memory[P].next) + static_cast<size_t>(memory[C].next)) <= maxSize)
                mergePC(); // merge P and C
            else
                standalone(); // C is standalone
        }
        else if (N != 0 && memory[N].status != 0)
        {
            if (static_cast<size_t>(memory[C].next) + static_cast<size_t>(memory[N].next) <= maxSize)
                mergeCN(); // merge C and N
            else
                standalone(); // standalone
        }
        else
        {
            // C is standalone
            standalone();
        }

        return true;
    }

#if DEBUG_ALLOCATOR
    std::cout << "IntrusiveAllocator::MemoryBlock::deallocate((" << ptr << ", " << size << ") OUTWITH block : " << this << std::endl;
#endif

    return false;
}

void IntrusiveAllocator::MemoryBlock::report(std::ostream& out) const
{
    out << "MemoryBlock " << this << " " << name << std::endl;
    out << "    alignment = " << alignment << std::endl;
    out << "    blockAlignment = " << blockAlignment << std::endl;
    out << "    blockSize = " << blockSize << ", memory = " << static_cast<void*>(memory) << std::endl;
    out << "    maximumAllocationSize = " << maximumAllocationSize << std::endl;
    out << "    firstSlot = " << firstSlot << std::endl;
    out << "    totalAvailableSize = " << totalAvailableSize() << std::endl;
    out << "    totalReservedSize = " << totalReservedSize() << std::endl;

    size_t position = firstSlot;
    while (position < capacity)
    {
        const auto& slot = memory[position];
        if (slot.status == 1)
        {
            out << "   memory[" << position << "] slot { " << slot.previous << ", " << slot.next << ", " << slot.status << "}, " << memory[position + 1].index << ", " << memory[position + 2].index << std::endl;
        }
        else
        {
            out << "   memory[" << position << "] slot { " << slot.previous << ", " << slot.next << ", " << slot.status << "} " << std::endl;
        }

        position += slot.next;
        if (slot.next == 0) break;
    }

    out << "   freeList.size() = " << freeLists.size() << " { " << std::endl;
    for (const auto& freeList : freeLists)
    {
        out << "   FreeList ( count = " << freeList.count << " , head = " << freeList.head << " ) {" << std::endl;

        size_t freePosition = freeList.head;
        while (freePosition != 0 && freePosition < capacity)
        {
            const auto& slot = memory[freePosition];
            out << "      slot " << freePosition << " { " << slot.previous << ", " << slot.next << ", " << slot.status
                << " } previous = " << memory[freePosition + 1].index << ", next = " << memory[freePosition + 2].index << std::endl;
            freePosition = memory[freePosition + 2].index;
        }

        out << "   }" << std::endl;
    }
}

bool IntrusiveAllocator::MemoryBlock::validate() const
{
    size_t previous = 0;
    size_t position = firstSlot;

    std::set<size_t> allocated;
    std::set<size_t> available;

    while (position < capacity)
    {
        const auto& slot = memory[position];
        if (slot.previous > capacity || slot.next > capacity)
        {
            std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " slot.corrupted invalid position = " << position << ", slot = {" << slot.previous << ", " << slot.next << ", " << int(slot.status) << "}" << std::endl;
            return false;
        }

        if (slot.status == 0)
            allocated.insert(position);
        else
            available.insert(position);

        if (slot.previous != 0)
        {
            if (slot.previous > position)
            {
                std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " slot.previous invalid position = " << position << ", slot = {" << slot.previous << ", " << slot.next << ", " << int(slot.status) << "}" << std::endl;
                return false;
            }
            size_t previous_position = position - slot.previous;
            if (previous_position != previous)
            {
                std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed : previous slot = " << previous << " doesn't match slot.previous, position = " << position << ", slot = {" << slot.previous << ", " << slot.next << ", " << int(slot.status) << "}" << std::endl;
                return false;
            }

            size_t previousFree = memory[position + 1].index;
            size_t nextFree = memory[position + 2].index;
            if (previousFree == position || nextFree == position)
            {
                std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed : slot's previous/nextFree points back to itself, position = " << position << ", slot = {" << slot.previous << ", " << slot.next << ", " << int(slot.status) << "} previousFree = " << previousFree << ", nextFree = " << nextFree << std::endl;
                return false;
            }
        }

        if (slot.next == 0)
        {
            std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed: position = " << position << " slot = {" << slot.previous << ", " << slot.next << ", " << static_cast<uint8_t>(slot.status) << "}" << std::endl;
            return false;
        }

        previous = position;
        position += slot.next;
    }

    std::set<size_t> inFreeList;

    // std::cout<<"No invalid entries found"<<std::endl;
    for (const auto& freeList : freeLists)
    {
        size_t previousPosition = 0;
        size_t freePosition = freeList.head;
        while (freePosition != 0 && freePosition < capacity)
        {
            const auto& slot = memory[freePosition];

            inFreeList.insert(freePosition);

            if (slot.status != 1)
            {
                std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed, non available slot in freeList, freePosition = " << freePosition << ", slot = {" << slot.previous << ", " << slot.next << ", " << static_cast<uint8_t>(slot.status) << "}" << std::endl;
                return false;
            }

            if (memory[freePosition + 1].index != previousPosition || memory[freePosition + 1].index == freePosition)
            {
                std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed, free list inconsisntent, head = " << freeList.head << ", previousPosition = " << previousPosition << ", freePosition = " << freePosition << ", slot = {" << slot.previous << ", " << slot.next << ", " << static_cast<uint8_t>(slot.status) << "} previousFree = " << memory[freePosition + 1].index << ", nextFree = " << memory[freePosition + 2].index << std::endl;
                return false;
            }

            previousPosition = freePosition;
            freePosition = memory[freePosition + 2].index;
        }
    }

    if (available.size() != inFreeList.size())
    {
        std::cerr << "IntrusiveAllocator::MemoryBlock::validate() " << this << " validation failed, Different number of entries in available and in freeList:  available.size() = " << available.size() << ", inFreeList.size() = " << inFreeList.size() << std::endl;
        return false;
    }

    return true;
}

size_t IntrusiveAllocator::MemoryBlock::totalAvailableSize() const
{
    size_t count = 0;
    size_t position = firstSlot;
    while (position < capacity)
    {
        const auto& slot = memory[position];
        position += slot.next;
        if (slot.status != 0) count += slot.next - 1;
    }

    return count * sizeof(Element);
}

size_t IntrusiveAllocator::MemoryBlock::totalReservedSize() const
{
    size_t count = 0;
    size_t position = firstSlot;
    while (position < capacity)
    {
        const auto& slot = memory[position];
        position += slot.next;
        if (slot.status == 0) count += slot.next - 1;
    }

    return count * sizeof(Element);
}

size_t IntrusiveAllocator::MemoryBlock::totalMemorySize() const
{
    size_t count = 0;
    size_t position = firstSlot;
    while (position < capacity)
    {
        const auto& slot = memory[position];
        position += slot.next;
        count += slot.next - 1;
    }

    return count * sizeof(Element);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryBlocks
//
IntrusiveAllocator::MemoryBlocks::MemoryBlocks(IntrusiveAllocator* in_parent, const std::string& in_name, size_t in_blockSize, size_t in_alignment) :
    parent(in_parent),
    name(in_name),
    alignment(in_alignment),
    blockSize(in_blockSize),
    maximumAllocationSize(IntrusiveAllocator::MemoryBlock::computeMaxiumAllocationSize(in_blockSize, in_alignment))
{
}

IntrusiveAllocator::MemoryBlocks::~MemoryBlocks()
{
}

void* IntrusiveAllocator::MemoryBlocks::allocate(std::size_t size)
{
    if (memoryBlockWithSpace)
    {
        auto ptr = memoryBlockWithSpace->allocate(size);
        if (ptr) return ptr;
    }

    size_t new_blockSize = std::max(size, blockSize);
    for (auto& block : memoryBlocks)
    {
        if (block != memoryBlockWithSpace)
        {
            auto ptr = block->allocate(size);
            if (ptr)
            {
                memoryBlockWithSpace = block;
                return ptr;
            }
        }
    }

    auto new_block = std::make_shared<MemoryBlock>(name, new_blockSize, alignment);
    if (parent)
    {
        parent->memoryBlocks[new_block->memory] = new_block;
    }

    if (memoryBlocks.empty())
    {
        maximumAllocationSize = new_block->maximumAllocationSize;
    }

    memoryBlockWithSpace = new_block;
    memoryBlocks.push_back(new_block);

    auto ptr = new_block->allocate(size);

    return ptr;
}

void IntrusiveAllocator::MemoryBlocks::report(std::ostream& out) const
{
    out << "IntrusiveAllocator::MemoryBlocks::report() memoryBlocks.size() = " << memoryBlocks.size() << std::endl;
    for (auto& memoryBlock : memoryBlocks)
    {
        memoryBlock->report(out);
    }
}

bool IntrusiveAllocator::MemoryBlocks::validate() const
{
    bool valid = true;
    for (auto& memoryBlock : memoryBlocks)
    {
        valid = memoryBlock->validate() && valid;
    }
    return valid;
}

size_t IntrusiveAllocator::MemoryBlocks::deleteEmptyMemoryBlocks()
{
    size_t count = 0;
    decltype(memoryBlocks) remainingBlocks;
    for (auto& memoryBlock : memoryBlocks)
    {
        if (memoryBlock->totalReservedSize() == 0)
        {
            count += memoryBlock->totalAvailableSize();
        }
        else
        {
            remainingBlocks.push_back(memoryBlock);
        }
    }
    memoryBlocks.swap(remainingBlocks);

    return count;
}

size_t IntrusiveAllocator::MemoryBlocks::totalAvailableSize() const
{
    size_t count = 0;
    for (auto& memoryBlock : memoryBlocks)
    {
        count += memoryBlock->totalAvailableSize();
    }
    return count;
}

size_t IntrusiveAllocator::MemoryBlocks::totalReservedSize() const
{
    size_t count = 0;
    for (auto& memoryBlock : memoryBlocks)
    {
        count += memoryBlock->totalReservedSize();
    }
    return count;
}

size_t IntrusiveAllocator::MemoryBlocks::totalMemorySize() const
{
    size_t count = 0;
    for (auto& memoryBlock : memoryBlocks)
    {
        count += memoryBlock->totalMemorySize();
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IntrusiveAllocator
//
IntrusiveAllocator::IntrusiveAllocator(size_t in_defaultAlignment) :
    Allocator(in_defaultAlignment)
{
    size_t Megabyte = size_t(1024) * size_t(1024);
    size_t blockSize = size_t(1) * Megabyte;

    allocatorMemoryBlocks.resize(vsg::ALLOCATOR_AFFINITY_LAST);
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_OBJECTS].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_OBJECTS", blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_DATA].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_DATA", size_t(16) * blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_NODES].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_NODES", blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_PHYSICS].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_PHYSICS", blockSize, 16));
}

IntrusiveAllocator::IntrusiveAllocator(std::unique_ptr<Allocator> in_nestedAllocator, size_t in_defaultAlignment) :
    Allocator(std::move(in_nestedAllocator), in_defaultAlignment)
{
    size_t Megabyte = size_t(1024) * size_t(1024);
    size_t blockSize = size_t(1) * Megabyte;

    allocatorMemoryBlocks.resize(vsg::ALLOCATOR_AFFINITY_LAST);
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_OBJECTS].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_OBJECTS", blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_DATA].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_DATA", size_t(16) * blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_NODES].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_NODES", blockSize, defaultAlignment));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_PHYSICS].reset(new MemoryBlocks(this, "ALLOCATOR_AFFINITY_PHYSICS", blockSize, 16));
}

IntrusiveAllocator::~IntrusiveAllocator()
{
}

void IntrusiveAllocator::setBlockSize(AllocatorAffinity allocatorAffinity, size_t blockSize)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorAffinity) < allocatorMemoryBlocks.size())
    {
        allocatorMemoryBlocks[allocatorAffinity]->blockSize = blockSize;
    }
    else
    {
        auto name = vsg::make_string("MemoryBlocks_", allocatorAffinity);

        allocatorMemoryBlocks.resize(allocatorAffinity + 1);
        allocatorMemoryBlocks[allocatorAffinity].reset(new MemoryBlocks(this, name, blockSize, defaultAlignment));
    }
}

void IntrusiveAllocator::report(std::ostream& out) const
{
    out << "IntrusiveAllocator::report() " << allocatorMemoryBlocks.size() << std::endl;

    for (const auto& memoryBlock : allocatorMemoryBlocks)
    {
        if (memoryBlock) memoryBlock->report(out);
    }

    validate();
}

void* IntrusiveAllocator::allocate(std::size_t size, AllocatorAffinity allocatorAffinity)
{
    std::scoped_lock<std::mutex> lock(mutex);

    // create a MemoryBlocks entry if one doesn't already exist
    if (allocatorAffinity > allocatorMemoryBlocks.size())
    {
        size_t blockSize = 1024 * 1024; // Megabyte
        allocatorMemoryBlocks.resize(allocatorAffinity + 1);
        allocatorMemoryBlocks[allocatorAffinity].reset(new MemoryBlocks(this, "MemoryBlockAffinity", blockSize, defaultAlignment));
    }

    void* ptr = nullptr;

    auto& blocks = allocatorMemoryBlocks[allocatorAffinity];
    if (blocks)
    {
        if (size <= blocks->maximumAllocationSize)
        {
            ptr = blocks->allocate(size);
            if (ptr) return ptr;
            //std::cout<<"IntrusiveAllocator::allocate() Failed to allocator memory from memoryBlocks "<<blocks.get()<<std::endl;
        }

        ptr = operator new(size, std::align_val_t{blocks->alignment});
        if (ptr) largeAllocations[ptr] = std::pair<size_t, size_t>(blocks->alignment, size);
        //std::cout<<"IntrusiveAllocator::allocate() MemoryBlocks aligned large allocation = "<<ptr<<" with size = "<<size<<", alignment = "<<blocks->alignment<<" blocks->maximumAllocationSize = "<<blocks->maximumAllocationSize<<std::endl;
        return ptr;
    }

    ptr = operator new(size, std::align_val_t{defaultAlignment});
    if (ptr) largeAllocations[ptr] = std::pair<size_t, size_t>(defaultAlignment, size);
    //std::cout<<"IntrusiveAllocator::allocate() default aligned large allocation = "<<ptr<<" with size = "<<size<<", alignment = "<<defaultAlignment<<std::endl;
    return ptr;
}

bool IntrusiveAllocator::deallocate(void* ptr, std::size_t size)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (memoryBlocks.empty()) return false;

    auto itr = memoryBlocks.upper_bound(ptr);
    if (itr != memoryBlocks.end())
    {
        if (itr != memoryBlocks.begin())
        {
            --itr;
            auto& block = itr->second;
            if (block->deallocate(ptr, size))
            {
                return true;
            }
        }
        else
        {
            auto& block = itr->second;
            if (block->deallocate(ptr, size))
            {
                return true;
            }
        }
    }
    else
    {
        auto& block = memoryBlocks.rbegin()->second;
        if (block->deallocate(ptr, size))
        {
            return true;
        }
    }

    auto la_itr = largeAllocations.find(ptr);
    if (la_itr != largeAllocations.end())
    {
        // large allocation;
        // std::cout<<"IntrusiveAllocator::deallocate("<<ptr<<") deleting large allocation."<<std::endl;
        operator delete(ptr, std::align_val_t{la_itr->second.first});
        largeAllocations.erase(la_itr);
        return true;
    }

    if (nestedAllocator && nestedAllocator->deallocate(ptr, size))
    {
        return true;
    }

    return false;
}

bool IntrusiveAllocator::validate() const
{
    bool valid = true;
    for (auto& memoryBlock : allocatorMemoryBlocks)
    {
        valid = memoryBlock->validate() && valid;
    }
    return valid;
}

size_t IntrusiveAllocator::deleteEmptyMemoryBlocks()
{
    size_t count = 0;
    for (auto& blocks : allocatorMemoryBlocks)
    {
        count += blocks->deleteEmptyMemoryBlocks();
    }
    return count;
}

size_t IntrusiveAllocator::totalAvailableSize() const
{
    size_t count = 0;
    for (auto& blocks : allocatorMemoryBlocks)
    {
        count += blocks->totalAvailableSize();
    }
    return count;
}

size_t IntrusiveAllocator::totalReservedSize() const
{
    size_t count = 0;
    for (auto& blocks : allocatorMemoryBlocks)
    {
        count += blocks->totalReservedSize();
    }
    return count;
}

size_t IntrusiveAllocator::totalMemorySize() const
{
    size_t count = 0;
    for (auto& blocks : allocatorMemoryBlocks)
    {
        count += blocks->totalMemorySize();
    }
    return count;
}
