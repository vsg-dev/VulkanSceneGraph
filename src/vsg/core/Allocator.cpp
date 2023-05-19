/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>

#include <algorithm>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator
//
Allocator::Allocator(std::unique_ptr<Allocator> in_nestedAllocator) :
    nestedAllocator(std::move(in_nestedAllocator))
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("Allocator()", this);
    }

    allocatorMemoryBlocks.resize(vsg::ALLOCATOR_AFFINITY_LAST);

    size_t Megabyte = 1024 * 1024;

    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_OBJECTS].reset(new MemoryBlocks(this, "MemoryBlocks_OBJECTS", size_t(Megabyte)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_DATA].reset(new MemoryBlocks(this, "MemoryBlocks_DATA", size_t(16 * Megabyte)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_AFFINITY_NODES].reset(new MemoryBlocks(this, "MemoryBlocks_NODES", size_t(Megabyte)));
}

Allocator::~Allocator()
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("~Allocator() ", this);
    }
}

std::unique_ptr<Allocator>& Allocator::instance()
{
    static std::unique_ptr<Allocator> s_allocator(new Allocator());
    return s_allocator;
}

void Allocator::report(std::ostream& out) const
{
    out << "Allocator::report() " << allocatorMemoryBlocks.size() << std::endl;
    out << "totalAvailableSize = " << totalAvailableSize() << ", totalReservedSize = " << totalReservedSize() << ", totalMemorySize = " << totalMemorySize() << std::endl;
    double totalReserved = static_cast<double>(totalReservedSize());

    std::scoped_lock<std::mutex> lock(mutex);
    for (const auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            size_t totalForBlock = memoryBlocks->totalReservedSize();
            out << memoryBlocks->name << " used = " << totalForBlock;
            if (totalReserved > 0.0)
            {
                out << ", " << (double(totalForBlock) / totalReserved) * 100.0 << "% of total used.";
            }
            out << std::endl;
        }
    }

    for (const auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            out << memoryBlocks->name << " " << memoryBlocks->memoryBlocks.size() << " blocks";
            for (const auto& value : memoryBlocks->memoryBlocks)
            {
                const auto& memorySlots = value.second->memorySlots;
                out << " [used = " << memorySlots.totalReservedSize() << ", avail = " << memorySlots.maximumAvailableSpace() << "]";
            }
            out << std::endl;
        }
    }
}

void* Allocator::allocate(std::size_t size, AllocatorAffinity allocatorAffinity)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
    {
        return operator new(size);
    }
    else if (allocatorType == ALLOCATOR_TYPE_MALLOC_FREE)
    {
        return std::malloc(size);
    }

    // create an MemoryBlocks entry if one doesn't already exist
    if (allocatorAffinity > allocatorMemoryBlocks.size())
    {
        if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
        {
            info("Allocator::allocate(", size, ", ", allocatorAffinity, ") out of bounds allocating new MemoryBlock");
        }

        auto name = make_string("MemoryBlocks_", allocatorAffinity);
        size_t blockSize = 1024 * 1024; // Megabyte

        allocatorMemoryBlocks.resize(allocatorAffinity + 1);
        allocatorMemoryBlocks[allocatorAffinity].reset(new MemoryBlocks(this, name, blockSize));
    }

    auto& memoryBlocks = allocatorMemoryBlocks[allocatorAffinity];
    if (memoryBlocks)
    {
        auto mem_ptr = memoryBlocks->allocate(size);
        if (mem_ptr)
        {
            if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
            {
                info("Allocated from MemoryBlock mem_ptr = ", mem_ptr, ", size = ", size, ", allocatorAffinity = ", int(allocatorAffinity));
            }
            return mem_ptr;
        }
    }

    void* ptr = Allocator::allocate(size, allocatorAffinity);
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("Allocator::allocate(", size, ", ", int(allocatorAffinity), ") ptr = ", ptr);
    }
    return ptr;
}

bool Allocator::deallocate(void* ptr, std::size_t size)
{
    std::scoped_lock<std::mutex> lock(mutex);

    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            if (memoryBlocks->deallocate(ptr, size))
            {
                if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
                {
                    info("Deallocated from MemoryBlock ", ptr);
                }
                return true;
            }
        }
    }

    if (nestedAllocator && nestedAllocator->deallocate(ptr, size)) return true;

    if (allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
    {
        operator delete(ptr);
        return true;
    }
    else if (allocatorType == ALLOCATOR_TYPE_MALLOC_FREE)
    {
        std::free(ptr);
        return true;
    }

    return false;
}

size_t Allocator::deleteEmptyMemoryBlocks()
{
    std::scoped_lock<std::mutex> lock(mutex);

    size_t memoryDeleted = 0;
    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks) memoryDeleted += memoryBlocks->deleteEmptyMemoryBlocks();
    }
    return memoryDeleted;
}

size_t Allocator::totalAvailableSize() const
{
    std::scoped_lock<std::mutex> lock(mutex);

    size_t size = 0;
    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks) size += memoryBlocks->totalAvailableSize();
    }
    return size;
}

size_t Allocator::totalReservedSize() const
{
    std::scoped_lock<std::mutex> lock(mutex);

    size_t size = 0;
    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks) size += memoryBlocks->totalReservedSize();
    }
    return size;
}

size_t Allocator::totalMemorySize() const
{
    std::scoped_lock<std::mutex> lock(mutex);

    size_t size = 0;
    for (auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks) size += memoryBlocks->totalMemorySize();
    }
    return size;
}

Allocator::MemoryBlocks* Allocator::getMemoryBlocks(AllocatorAffinity allocatorAffinity)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorAffinity) < allocatorMemoryBlocks.size()) return allocatorMemoryBlocks[allocatorAffinity].get();
    return {};
}

Allocator::MemoryBlocks* Allocator::getOrCreateMemoryBlocks(AllocatorAffinity allocatorAffinity, const std::string& name, size_t blockSize)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorAffinity) < allocatorMemoryBlocks.size())
    {
        allocatorMemoryBlocks[allocatorAffinity]->name = name;
        allocatorMemoryBlocks[allocatorAffinity]->blockSize = blockSize;
    }
    else
    {
        allocatorMemoryBlocks.resize(allocatorAffinity + 1);
        allocatorMemoryBlocks[allocatorAffinity].reset(new MemoryBlocks(this, name, blockSize));
    }
    return allocatorMemoryBlocks[allocatorAffinity].get();
}

void Allocator::setBlockSize(AllocatorAffinity allocatorAffinity, size_t blockSize)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorAffinity) < allocatorMemoryBlocks.size())
    {
        allocatorMemoryBlocks[allocatorAffinity]->blockSize = blockSize;
    }
    else
    {
        auto name = make_string("MemoryBlocks_", allocatorAffinity);

        allocatorMemoryBlocks.resize(allocatorAffinity + 1);
        allocatorMemoryBlocks[allocatorAffinity].reset(new MemoryBlocks(this, name, blockSize));
    }
}

void Allocator::setMemoryTracking(int mt)
{
    memoryTracking = mt;
    for (auto& amb : allocatorMemoryBlocks)
    {
        if (amb)
        {
            for (auto& value : amb->memoryBlocks)
            {
                value.second->memorySlots.memoryTracking = mt;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator::MemoryBlock
//
Allocator::MemoryBlock::MemoryBlock(size_t blockSize, int memoryTracking, AllocatorType in_allocatorType) :
    memorySlots(blockSize, memoryTracking),
    allocatorType(in_allocatorType)
{
    if (allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
    {
        memory = static_cast<uint8_t*>(operator new(blockSize));
    }
    else
    {
        memory = static_cast<uint8_t*>(std::malloc(blockSize));
    }

    if (memorySlots.memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemoryBlock(", blockSize, ") allocated memory");
    }
}

Allocator::MemoryBlock::~MemoryBlock()
{
    if (memorySlots.memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemoryBlock::~MemoryBlock(", memorySlots.totalMemorySize(), ") freed memory");
    }

    if (allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
    {
        operator delete(memory);
    }
    else
    {
        std::free(memory);
    }
}

void* Allocator::MemoryBlock::allocate(std::size_t size)
{
    auto [allocated, offset] = memorySlots.reserve(size, 4);
    if (allocated)
        return memory + offset;
    else
        return nullptr;
}

bool Allocator::MemoryBlock::deallocate(void* ptr, std::size_t size)
{
    if (ptr >= memory)
    {
        size_t offset = static_cast<uint8_t*>(ptr) - memory;
        if (offset < memorySlots.totalMemorySize())
        {
            if (!memorySlots.release(offset, size))
            {
                warn("Allocator::MemoryBlock::deallocate(", ptr, ") problem - couldn't release");
            }
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator::MemoryBlocks
//
Allocator::MemoryBlocks::MemoryBlocks(Allocator* in_parent, const std::string& in_name, size_t in_blockSize) :
    parent(in_parent),
    name(in_name),
    blockSize(in_blockSize)
{
    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("Allocator::MemoryBlocks::MemoryBlocks(", parent, ", ", name, ", ", blockSize, ")");
    }
}

Allocator::MemoryBlocks::~MemoryBlocks()
{
    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemoryBlocks::~MemoryBlocks() name = ", name, ", ", memoryBlocks.size());
    }
}

void* Allocator::MemoryBlocks::allocate(std::size_t size)
{
    if (latestMemoryBlock)
    {
        auto ptr = latestMemoryBlock->allocate(size);
        if (ptr) return ptr;
    }

    // search existing blocks from last to first for space for the required memory allocation.
    for (auto itr = memoryBlocks.rbegin(); itr != memoryBlocks.rend(); ++itr)
    {
        auto& block = itr->second;
        if (block != latestMemoryBlock)
        {
            auto ptr = block->allocate(size);
            if (ptr) return ptr;
        }
    }

    size_t new_blockSize = std::max(size, blockSize);

    auto block = std::make_shared<MemoryBlock>(new_blockSize, parent->memoryTracking, parent->memoryBlocksAllocatorType);
    latestMemoryBlock = block;

    auto ptr = block->allocate(size);

    memoryBlocks[block->memory] = std::move(block);

    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("Allocator::MemoryBlocks::allocate(", size, ") MemoryBlocks.name = ", name, ", allocated in new MemoryBlock ", parent->memoryTracking);
    }

    return ptr;
}

bool Allocator::MemoryBlocks::deallocate(void* ptr, std::size_t size)
{
    if (memoryBlocks.empty()) return false;

    auto itr = memoryBlocks.upper_bound(ptr);
    if (itr != memoryBlocks.end())
    {
        if (itr != memoryBlocks.begin())
        {
            --itr;
            auto& block = itr->second;
            if (block->deallocate(ptr, size)) return true;
        }
        else
        {
            auto& block = itr->second;
            if (block->deallocate(ptr, size)) return true;
        }
    }
    else
    {
        auto& block = memoryBlocks.rbegin()->second;
        if (block->deallocate(ptr, size)) return true;
    }

    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemoryBlocks:deallocate() MemoryBlocks.name = ", name, ",  couldn't locate pointer to deallocate ", ptr);
    }
    return false;
}

size_t Allocator::MemoryBlocks::deleteEmptyMemoryBlocks()
{
    size_t memoryDeleted = 0;
    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemoryBlocks:deleteEmptyMemoryBlocks() MemoryBlocks.name = ", name);
    }

    auto itr = memoryBlocks.begin();
    while (itr != memoryBlocks.end())
    {
        auto& block = itr->second;
        if (block->memorySlots.empty())
        {
            if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
            {
                info("    MemoryBlocks:deleteEmptyMemoryBlocks() MemoryBlocks.name = ", name, ",  removing MemoryBlock", block.get());
            }
            if (block == latestMemoryBlock) latestMemoryBlock = nullptr;
            memoryDeleted += block->memorySlots.totalMemorySize();
            itr = memoryBlocks.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
    return memoryDeleted;
}

size_t Allocator::MemoryBlocks::totalAvailableSize() const
{
    size_t size = 0;
    for (auto& value : memoryBlocks)
    {
        size += value.second->memorySlots.totalAvailableSize();
    }
    return size;
}

size_t Allocator::MemoryBlocks::totalReservedSize() const
{
    size_t size = 0;
    for (auto& value : memoryBlocks)
    {
        size += value.second->memorySlots.totalReservedSize();
    }
    return size;
}

size_t Allocator::MemoryBlocks::totalMemorySize() const
{
    size_t size = 0;
    for (auto& value : memoryBlocks)
    {
        size += value.second->memorySlots.totalMemorySize();
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::allocate and vsg::deallocate convenience functions that map to using the Allocator singleton.
//
void* vsg::allocate(std::size_t size, AllocatorAffinity allocatorAffinity)
{
    return Allocator::instance()->allocate(size, allocatorAffinity);
}

void vsg::deallocate(void* ptr, std::size_t size)
{
    Allocator::instance()->deallocate(ptr, size);
}
