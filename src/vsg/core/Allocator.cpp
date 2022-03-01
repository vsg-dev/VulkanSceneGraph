/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Exception.h>

#include <algorithm>
#include <iostream>

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
        std::cout << "Allocator()" << this << std::endl;
    }

    allocatorMemoryBlocks.resize(vsg::ALLOCATOR_LAST);

    // TODO need to set to a more sensible default
    allocatorMemoryBlocks[vsg::ALLOCATOR_OBJECTS].reset(new MemoryBlocks(this, "ALLOCATOR_OBJECTS", size_t(4096)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_DATA].reset(new MemoryBlocks(this, "ALLOCATOR_DATA", size_t(2048)));
    allocatorMemoryBlocks[vsg::ALLOCATOR_NODES].reset(new MemoryBlocks(this, "ALLOCATOR_NODES", size_t(512)));
}

Allocator::~Allocator()
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout << "~Allocator() " << this << std::endl;
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
    double totalReserved(totalReservedSize());

    std::scoped_lock<std::mutex> lock(mutex);
    for (const auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            size_t totalForBlock = memoryBlocks->totalReservedSize();
            out <<memoryBlocks->name << " used = " << totalForBlock;
            if (totalReserved>0.0)
            {
                out<<", "<< (double(totalForBlock)/totalReserved)*100.0 << "% of total used.";
            }
            out << std::endl;
        }
    }

    for (const auto& memoryBlocks : allocatorMemoryBlocks)
    {
        if (memoryBlocks)
        {
            out <<memoryBlocks->name << " "<<memoryBlocks->memoryBlocks.size() << " blocks";
            for (const auto& memoryBlock : memoryBlocks->memoryBlocks)
            {
                const auto& memorySlots = memoryBlock->memorySlots;
                out << " [used = " << memorySlots.totalReservedSize() << ", avail = " << memorySlots.maximumAvailableSpace() << "]";
            }
            out << std::endl;
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
            if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
            {
                std::cout << "Allocated from MemoryBlock mem_ptr = " << mem_ptr << ", size = " << size << ", allocatorType = " << int(allocatorType) << std::endl;
            }
            return mem_ptr;
        }
    }

    void* ptr = Allocator::allocate(size, allocatorType);
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout << "Allocator::allocate(" << size << ", " << int(allocatorType) << ") ptr = " << ptr << std::endl;
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
                    std::cout<< "Deallocated from MemoryBlock " << ptr << std::endl;
                }
                return true;
            }
        }
    }

    if (nestedAllocator) return nestedAllocator->deallocate(ptr, size);

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

Allocator::MemoryBlocks* Allocator::getMemoryBlocks(AllocatorType allocatorType)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorType) < allocatorMemoryBlocks.size()) return allocatorMemoryBlocks[allocatorType].get();
    return {};
}

Allocator::MemoryBlocks* Allocator::getOrCreateMemoryBlocks(AllocatorType allocatorType, const std::string& name, size_t blockSize)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorType) < allocatorMemoryBlocks.size())
    {
        allocatorMemoryBlocks[allocatorType]->name = name;
        allocatorMemoryBlocks[allocatorType]->blockSize = blockSize;
    }
    else
    {
        allocatorMemoryBlocks.resize(allocatorType+1);
        allocatorMemoryBlocks[allocatorType].reset(new MemoryBlocks(this, name, blockSize));
    }
    return allocatorMemoryBlocks[allocatorType].get();
}

void Allocator::setBlockSize(AllocatorType allocatorType, size_t blockSize)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (size_t(allocatorType) < allocatorMemoryBlocks.size())
    {
        allocatorMemoryBlocks[allocatorType]->blockSize = blockSize;
    }
    else
    {
        std::string name = "UnamedMemoryBlock";

        allocatorMemoryBlocks.resize(allocatorType+1);
        allocatorMemoryBlocks[allocatorType].reset(new MemoryBlocks(this, name, blockSize));
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::Allocator::MemoryBlock
//
Allocator::MemoryBlock::MemoryBlock(size_t blockSize, int memoryTracking) :
    memorySlots(blockSize, memoryTracking)
{
    memory = static_cast<uint8_t*>(std::malloc(blockSize));

    if (memorySlots.memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout << "MemoryBlock(" << blockSize << ") allocatoed memory" << std::endl;
    }
}

Allocator::MemoryBlock::~MemoryBlock()
{
    if (memorySlots.memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout << "MemoryBlock::~MemoryBlock(" << memorySlots.totalMemorySize() << ") freed memory" << std::endl;
    }

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

bool Allocator::MemoryBlock::deallocate(void* ptr, std::size_t size)
{
    if (ptr >= memory)
    {
        size_t offset = static_cast<uint8_t*>(ptr) - memory;
        if (offset < memorySlots.totalMemorySize())
        {
            if (!memorySlots.release(offset, size))
            {
                std::cout<<"Allocator::MemoryBlock::deallocate("<<ptr<<") problem - couldn't release"<<std::endl;
            }
            return true;
        }
    }
    return false;
}

void Allocator::setMemoryTracking(int mt)
{
    std::cout<<"Allocator::setMemoryTracking("<<mt<<")"<<std::endl;
    memoryTracking = mt;
    for(auto& amb : allocatorMemoryBlocks)
    {
        if (amb)
        {
            for(auto& mb : amb->memoryBlocks)
            {
                mb->memorySlots.memoryTracking = mt;
                std::cout<<"    mb->memorySlots.memoryTracking = "<<mb->memorySlots.memoryTracking<<std::endl;
            }
        }
    }
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
        std::cout << "Allocator::MemoryBlocks::MemoryBlocks("<<parent<<", "<<name<<", " << blockSize << ")"<<std::endl;
    }
}

Allocator::MemoryBlocks::~MemoryBlocks()
{
    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout << "MemoryBlocks::~MemoryBlocks() name = " << name << ", " << memoryBlocks.size() << std::endl;
    }
}

void* Allocator::MemoryBlocks::allocate(std::size_t size)
{
    for (auto& block : memoryBlocks)
    {
        auto ptr = block->allocate(size);
        if (ptr) return ptr;
    }

    size_t new_blockSize = std::max(size, blockSize);

    std::unique_ptr<MemoryBlock> block(new MemoryBlock(new_blockSize, parent->memoryTracking));
    auto ptr = block->allocate(size);

    memoryBlocks.push_back(std::move(block));

    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout<<"Allocator::MemoryBlocks::allocate("<<size<<") MemoryBlocks.name = "<<name<<", allocated in new MemoryBlock "<<parent->memoryTracking<<std::endl;
    }

    return ptr;
}

bool Allocator::MemoryBlocks::deallocate(void* ptr, std::size_t size)
{
    for (auto& block : memoryBlocks)
    {
        if (block->deallocate(ptr, size)) return true;
    }

    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout<< "MemoryBlocks:deallocate() MemoryBlocks.name = "<<name<<",  couldn't locate pointer to deallocate " << ptr << std::endl;
    }
    return false;
}

size_t Allocator::MemoryBlocks::deleteEmptyMemoryBlocks()
{
    size_t memoryDeleted = 0;
    if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout<< "MemoryBlocks:deleteEmptyMemoryBlocks() MemoryBlocks.name = "<<name<< std::endl;
    }

    auto itr = memoryBlocks.begin();
    while(itr != memoryBlocks.end())
    {
        auto& block = *itr;
        if (block->memorySlots.empty())
        {
            if (parent->memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
            {
                std::cout<< "    MemoryBlocks:deleteEmptyMemoryBlocks() MemoryBlocks.name = "<<name<<",  removing MemoryBlock" <<block.get()<< std::endl;
            }
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
    for (auto& block : memoryBlocks)
    {
        size += block->memorySlots.totalAvailableSize();
    }
    return size;
}

size_t Allocator::MemoryBlocks::totalReservedSize() const
{
    size_t size = 0;
    for (auto& block : memoryBlocks)
    {
        size += block->memorySlots.totalReservedSize();
    }
    return size;
}

size_t Allocator::MemoryBlocks::totalMemorySize() const
{
    size_t size = 0;
    for (auto& block : memoryBlocks)
    {
        size += block->memorySlots.totalMemorySize();
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::allocate and vsg::decalloate convinience functions that map to using the Allocator singleton.
//
void* vsg::allocate(std::size_t size, AllocatorType allocatorType)
{
    return Allocator::instance()->allocate(size, allocatorType);
}

void vsg::deallocate(void* ptr, std::size_t size)
{
    Allocator::instance()->deallocate(ptr, size);
}
