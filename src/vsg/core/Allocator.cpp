/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/io/Options.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

#if 0
#    define DEBUG \
        if (true) std::cout
#else
#    define DEBUG \
        if (false) std::cout
#endif

///////////////////////////////////////////////////////////////////////////////
//
// MemorySlots
//
MemorySlots::MemorySlots(size_t availableMemorySize)
{
    _availableMemory.insert(SizeOffset(availableMemorySize, 0));
    _offsetSizes.insert(OffsetSize(0, availableMemorySize));

    _totalMemorySize = availableMemorySize;
}

size_t MemorySlots::totalAvailableSize() const
{
    size_t totalSize = 0;
    for (const auto& sizeOffset : _availableMemory)
    {
        totalSize += sizeOffset.first;
    }
    return totalSize;
}

size_t MemorySlots::totalReservedSize() const
{
    size_t totalSize = 0;
    for (const auto& sizeOffset : _reservedOffsetSizes)
    {
        totalSize += sizeOffset.second;
    }
    return totalSize;
}

bool MemorySlots::check() const
{
    if (_availableMemory.size() != _offsetSizes.size())
    {
        std::cout << "Warning: MemorySlots::check() _availableMemory.size() " << _availableMemory.size() << " != _offsetSizes.size() " << _offsetSizes.size() << std::endl;
    }

    size_t availableSize = 0;
    for (auto& offsetSize : _offsetSizes)
    {
        availableSize += offsetSize.second;
    }

    size_t reservedSize = 0;
    for (auto& offsetSize : _reservedOffsetSizes)
    {
        reservedSize += offsetSize.second;
    }

    size_t computedSize = availableSize + reservedSize;
    if (computedSize != _totalMemorySize)
    {
        std::cout << "Warning : MemorySlots::check() " << this << " failed, computeedSize (" << computedSize << ") != _totalMemorySize (" << _totalMemorySize << ")" << std::endl;

        report();

        throw "MemorySlots check failed";

        return false;
    }

    return true;
}

void MemorySlots::report() const
{
    std::cout << "MemorySlots::report()" << std::endl;
    for (auto& [offset, size] : _offsetSizes)
    {
        std::cout << "    available " << offset << ", " << size << std::endl;
    }

    for (auto& [offset, size] : _reservedOffsetSizes)
    {
        std::cout << "    reserved " << std::dec << offset << ", " << size << std::endl;
    }
}

MemorySlots::OptionalOffset MemorySlots::reserve(size_t size, size_t alignment)
{
    if (full()) return OptionalOffset(false, 0);

    auto itr = _availableMemory.lower_bound(size);
    while (itr != _availableMemory.end())
    {
        SizeOffset slot(*itr);
        size_t slotStart = slot.second;
        size_t slotSize = slot.first;
        size_t slotEnd = slotStart + slotSize;

        size_t alignedStart = ((slotStart + alignment - 1) / alignment) * alignment;
        size_t alignedEnd = alignedStart + size;
        size_t alignedSize = alignedEnd - slotStart;

        if (alignedSize <= slotSize)
        {
            // remove slot
            _availableMemory.erase(itr);
            if (auto offsetSize_itr = _offsetSizes.find(slotStart); offsetSize_itr != _offsetSizes.end()) _offsetSizes.erase(offsetSize_itr);

            // check if there the front of the slot isn't used completely, if so generate an available space for it.
            if (alignedStart > slotStart)
            {
                // insert new slot with previous slots start and new end.
                size_t preAlignedStartSize = alignedStart - slotStart;
                _availableMemory.insert(SizeOffset(preAlignedStartSize, slotStart));
                _offsetSizes.insert(OffsetSize(slotStart, preAlignedStartSize));
            }

            // check if there is space at the end slot that isn't used completely, if so generate an available space for it.
            if (alignedEnd < slotEnd)
            {
                // insert new slot with new end and new size
                size_t postAlignedEndSize = slotEnd - alignedEnd;
                _availableMemory.insert(SizeOffset(postAlignedEndSize, alignedEnd));
                _offsetSizes.insert(OffsetSize(alignedEnd, postAlignedEndSize));
            }

            _reservedOffsetSizes[alignedStart] = (alignedEnd - alignedStart);

#if DO_CHECK
            check();
#endif

            return OptionalOffset(true, alignedStart);
        }
        else
        {
            // std::cout << "    Slot slotStart = " << slotStart << ", slotSize = " << slotSize << " not big enough once for request size = " << size << std::endl;
            ++itr;
        }
    }

    //std::cout<<"MemorySlots::reserve("<<std::dec<<size<<") with alingment "<<alignment<<" No slots available for this size, biggest available slot is : "<<_availableMemory.rbegin()->first<<std::endl;
    //report();

    return OptionalOffset(false, 0);
}

void MemorySlots::release(size_t offset, size_t size)
{
    auto reserved_itr = _reservedOffsetSizes.find(offset);
    if (reserved_itr == _reservedOffsetSizes.end())
    {
#if DO_CHECK
        std::cout << "   MemorySlots::release() slot not found" << std::endl;
#endif
        return;
    }
    else
    {
#if DO_CHECK
        if (reserved_itr->second != size)
        {
            std::cout << "    MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second = " << std::dec << reserved_itr->second << ", size=" << size << std::endl;
            if (size != 0) throw "MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second";
        }
        else
        {
            std::cout << "    MemorySlots::release() slot found " << std::endl;
        }
#endif

        size = reserved_itr->second;

        _reservedOffsetSizes.erase(reserved_itr);
    }

    if (_offsetSizes.empty())
    {
        // first empty space
        _availableMemory.insert(SizeOffset(size, offset));
        _offsetSizes.insert(OffsetSize(offset, size));

#if DO_CHECK
        check();
#endif
        return;
    }

    // need to find adjacent blocks before and after to see if we can join them together options are:
    //    abutes to neither before or after
    //    abutes to before, so replace before with new combined length
    //    abutes to after, so remove after entry and insert new entry with combined length
    //    abutes to both before and after, so replace before with newly combined length of all three, remove after entry

    auto slotAfter = _offsetSizes.upper_bound(offset);

    auto slotBefore = slotAfter;
    if (slotBefore != _offsetSizes.end())
    {
        if (slotBefore == _offsetSizes.begin())
        {
            slotBefore = _offsetSizes.end();
        }
        else
        {
            --slotBefore;
        }
    }
    else
    {
        slotBefore = _offsetSizes.rbegin().base();
    }

    auto eraseSlot = [&](OffsetSizes::iterator offsetSizeItr) {
        auto range = _availableMemory.equal_range(offsetSizeItr->second);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == offsetSizeItr->first)
            {
                _availableMemory.erase(itr);
                _offsetSizes.erase(offsetSizeItr);
                break;
            }
        }
    };

    if (slotBefore != _offsetSizes.end())
    {
        size_t endOfBeforeSlot = slotBefore->first + slotBefore->second;

        if (endOfBeforeSlot == offset)
        {
            size_t endOfReleasedSlot = offset + size;
            size_t totalSizeOfMergedSlots = endOfReleasedSlot - slotBefore->first;

            offset = slotBefore->first;
            size = totalSizeOfMergedSlots;

            eraseSlot(slotBefore);
        }
    }
    if (slotAfter != _offsetSizes.end())
    {
        size_t endOfReleasedSlot = offset + size;

        if (endOfReleasedSlot == slotAfter->first)
        {
            size_t endOfSlotAfter = slotAfter->first + slotAfter->second;
            size_t totalSizeOfMergedSlots = endOfSlotAfter - offset;
            size = totalSizeOfMergedSlots;

            eraseSlot(slotAfter);
        }
    }

    _availableMemory.insert(SizeOffset(size, offset));
    _offsetSizes.insert(OffsetSize(offset, size));

    //report();

#if DO_CHECK
    check();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator
//
Allocator::Allocator(std::unique_ptr<Allocator> in_nestedAllocator) :
    nestedAllocator(std::move(in_nestedAllocator))
{
    DEBUG << "Allocator()" << this << std::endl;

    allocatorMemoryBlocks.resize(vsg::ALLOCATOR_LAST);

    // TODO need to set to a more sensible default
    allocatorMemoryBlocks[vsg::ALLOCATOR_OBJECTS].reset(new MemoryBlocks("ALLOCATOR_OBJECTS", size_t(1024 * 1024 * 16)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_DATA].reset(new MemoryBlocks("ALLOCATOR_DATA", size_t(1024 * 1024 * 16)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_NODES].reset(new MemoryBlocks("ALLOCATOR_NODES", size_t(1024 * 1024 * 16)));
}

Allocator::~Allocator()
{
    DEBUG << "~Allocator() " << this << std::endl;
}

std::unique_ptr<Allocator>& Allocator::instance()
{
    static std::unique_ptr<Allocator> s_allocator(new Allocator());
    return s_allocator;
}

void Allocator::report() const
{
    DEBUG << "Allocator::report() " << allocatorMemoryBlocks.size() << std::endl;
    std::scoped_lock<std::mutex> lock(mutex);
    for (const auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            DEBUG << "    " << memoryBlocks->name;
            for (const auto& memoryBlock : memoryBlocks->memoryBlocks)
            {
                const auto& memorySlots = memoryBlock->memorySlots;
                DEBUG << ", [" << memorySlots.totalReservedSize() << ", " << memorySlots.maximumAvailableSpace() << "]";
            }
            DEBUG << std::endl;
        }
    }
}

void* Allocator::allocate(std::size_t size, vsg::AllocatorType allocatorType)
{
    std::scoped_lock<std::mutex> lock(mutex);

    auto& memoryBlocks = allocatorMemoryBlocks[allocatorType];
    if (memoryBlocks)
    {
        auto mem_ptr = memoryBlocks->allocate(size);
        if (mem_ptr)
        {
            DEBUG << "Allocated from MemoryBlock mem_ptr = " << mem_ptr << ", size = " << size << ", allocatorType = " << int(allocatorType) << std::endl;
            return mem_ptr;
        }
    }

    void* ptr = Allocator::allocate(size, allocatorType);
    DEBUG << "Allocator::allocate(" << size << ", " << int(allocatorType) << ") ptr = " << ptr << std::endl;
    return ptr;
}

bool Allocator::deallocate(void* ptr)
{
    std::scoped_lock<std::mutex> lock(mutex);

    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            if (memoryBlocks->deallocate(ptr))
            {
                DEBUG << "Deallocated from MemoryBlock " << ptr << std::endl;
                return true;
            }
        }
    }

    if (nestedAllocator) return nestedAllocator->deallocate(ptr);

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator::MemoryBlock
//
Allocator::MemoryBlock::MemoryBlock(size_t blockSize) :
    memorySlots(blockSize)
{
    memory = static_cast<uint8_t*>(std::malloc(blockSize));

    DEBUG << "MemoryBlock(" << blockSize << ") allocatoed memory" << std::endl;
}

Allocator::MemoryBlock::~MemoryBlock()
{
    DEBUG << "MemoryBlock::~MemoryBlock(" << memorySlots.totalMemorySize() << ") freed memory" << std::endl;
    // memorySlots.report();
    std::free(memory);
}

void* Allocator::MemoryBlock::allocate(std::size_t size)
{
    auto [allocated, offset] = memorySlots.reserve(size, 4);
    if (allocated)
        return memory + offset;
    else
        return nullptr;
}

bool Allocator::MemoryBlock::deallocate(void* ptr)
{
    if (ptr >= memory)
    {
        size_t offset = static_cast<uint8_t*>(ptr) - memory;
        if (offset < memorySlots.totalMemorySize())
        {
            memorySlots.release(offset, 0);
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator::MemoryBlocks
//
Allocator::MemoryBlocks::MemoryBlocks(const std::string& in_name, size_t in_blockSize) :
    name(in_name),
    blockSize(in_blockSize)
{
}

Allocator::MemoryBlocks::~MemoryBlocks()
{
    DEBUG << "MemoryBlocks::~MemoryBlocks() name = " << name << ", " << memoryBlocks.size() << std::endl;
}

void* Allocator::MemoryBlocks::allocate(std::size_t size)
{
    for (auto& block : memoryBlocks)
    {
        auto ptr = block->allocate(size);
        if (ptr) return ptr;
    }

    size_t new_blockSize = std::max(size, blockSize);

    std::unique_ptr<MemoryBlock> block(new MemoryBlock(new_blockSize));
    auto ptr = block->allocate(size);

    memoryBlocks.push_back(std::move(block));

    return ptr;
}

bool Allocator::MemoryBlocks::deallocate(void* ptr)
{
    for (auto& block : memoryBlocks)
    {
        if (block->deallocate(ptr)) return true;
    }

    DEBUG << "MemoryBlocks:deallocate() : couldn't locate point to deallocato " << ptr << std::endl;
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::allocate and vsg::decalloate convinience functions that map to using the Allocator singleton.
//
void* vsg::allocate(std::size_t size, AllocatorType allocatorType)
{
    return Allocator::instance()->allocate(size, allocatorType);
}

void vsg::deallocate(void* ptr)
{
    Allocator::instance()->deallocate(ptr);
}
