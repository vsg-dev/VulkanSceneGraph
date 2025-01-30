/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/MemorySlots.h>
#include <vsg/io/Logger.h>

#include <algorithm>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////
//
// MemorySlots
//
MemorySlots::MemorySlots(size_t availableMemorySize, int in_memoryTracking) :
    memoryTracking(in_memoryTracking)
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemorySlots::MemorySlots(", availableMemorySize, ") ", this);
    }

    insertAvailableSlot(0, availableMemorySize);

    _totalMemorySize = availableMemorySize;
}

MemorySlots::~MemorySlots()
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        if (_availableMemory.size() == 1)
        {
            info("MemorySlots::~MemorySlots() ", this, ", all slots restored correctly.");
        }
        else
        {
            info("MemorySlots::~MemorySlots() ", this, ", not all slots restored correctly.");
            info_stream([&](auto& fout) { report(fout); });
        }
    }
    if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS)
    {
        check();
    }
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
    for (const auto& sizeOffset : _reservedMemory)
    {
        totalSize += sizeOffset.second;
    }
    return totalSize;
}

bool MemorySlots::check() const
{
    if (_availableMemory.size() != _offsetSizes.size())
    {
        warn("MemorySlots::check() _availableMemory.size() ", _availableMemory.size(), " != _offsetSizes.size() ", _offsetSizes.size());
    }

    size_t availableSize = 0;
    for (const auto& offsetSize : _offsetSizes)
    {
        availableSize += offsetSize.second;
    }

    size_t reservedSize = 0;
    for (const auto& offsetSize : _reservedMemory)
    {
        reservedSize += offsetSize.second;
    }

    size_t computedSize = availableSize + reservedSize;
    if (computedSize != _totalMemorySize)
    {
        warn("MemorySlots::check() ", this, " failed, computedSize (", computedSize, ") != _totalMemorySize (", _totalMemorySize, ")");
        warn_stream([&](auto& fout) { report(fout); });

        return false;
    }

    return true;
}

void MemorySlots::report(std::ostream& out) const
{
    out << "MemorySlots::report() " << this << std::endl;
    for (auto& [offset, size] : _offsetSizes)
    {
        out << "    available " << offset << ", " << size << std::endl;
    }

    for (auto& [offset, size] : _reservedMemory)
    {
        out << "    reserved " << std::dec << offset << ", " << size << std::endl;
    }
}

void MemorySlots::insertAvailableSlot(size_t offset, size_t size)
{
    _offsetSizes.emplace(offset, size);
    _availableMemory.emplace(size, offset);
}

void MemorySlots::removeAvailableSlot(size_t offset, size_t size)
{
    _offsetSizes.erase(offset);
    auto end = _availableMemory.upper_bound(size);
    for (auto itr = _availableMemory.lower_bound(size); itr != end; ++itr)
    {
        if (itr->second == offset)
        {
            _availableMemory.erase(itr);
            break;
        }
    }
}

MemorySlots::OptionalOffset MemorySlots::reserve(size_t size, size_t alignment)
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("\nMemorySlots::reserve(", size, ", ", alignment, ") ", this);
    }

    if (full()) return OptionalOffset(false, 0);

    auto itr = _availableMemory.lower_bound(size);
    while (itr != _availableMemory.end())
    {
        size_t slotSize = itr->first;
        size_t slotStart = itr->second;
        size_t slotEnd = slotStart + slotSize;
        size_t alignedStart = ((slotStart + alignment - 1) / alignment) * alignment;
        size_t alignedEnd = alignedStart + size;
        if (alignedEnd <= slotEnd) // slot big enough
        {
            // remove available slot
            removeAvailableSlot(slotStart, slotSize);

            if (slotStart < alignedStart) // space before newly reserved slot
            {
                insertAvailableSlot(slotStart, alignedStart - slotStart);
            }

            if (alignedEnd < slotEnd) // space after newly reserved slot
            {
                slotStart = alignedEnd;
                insertAvailableSlot(slotStart, slotEnd - slotStart);
            }

            // record and return reserved slot
            _reservedMemory.emplace(alignedStart, size);

            if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
            {
                info("MemorySlots::reserve(", size, ", ", alignment, ") ", this, " allocated [", alignedStart, ", ", size, "]");
            }

            if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS) check();

            return {true, alignedStart};
        }
        else // slot not big enough so advance to the next slot
        {
            ++itr;
        }
    }

    if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS) check();

    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("MemorySlots::reserve(", size, ", ", alignment, ") ", this, " no suitable slots found");
    }
    return {false, 0};
}

bool MemorySlots::release(size_t offset, size_t size)
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        info("\nMemorySlots::release(", offset, ", ", size, ") ", this);
    }

    auto itr = _reservedMemory.find(offset);
    if (itr == _reservedMemory.end())
    {
        // entry isn't in reserved slots
        return false;
    }

    if (size != itr->second)
    {
        if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
        {
            info("    reserved slot different size = ", size, ", itr->second = ", itr->second);
        }

        size = itr->second;
    }

    // remove from reserved list
    _reservedMemory.erase(itr);

    if (_offsetSizes.empty())
    {
        insertAvailableSlot(offset, size);

        if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS) check();

        return true;
    }

    size_t slotStart = offset;
    size_t slotEnd = offset + size;

    auto next_slot_itr = _offsetSizes.lower_bound(slotStart);
    if (next_slot_itr != _offsetSizes.end())
    {
        if (next_slot_itr != _offsetSizes.begin())
        {
            auto prev_slot_itr = next_slot_itr;
            --prev_slot_itr;

            size_t prev_slotEnd = prev_slot_itr->first + prev_slot_itr->second;
            if (prev_slotEnd == slotStart)
            {
                // previous slot abuts with the one being released so remove it.
                slotStart = prev_slot_itr->first;
                removeAvailableSlot(prev_slot_itr->first, prev_slot_itr->second);
            }
        }

        if (next_slot_itr->first == slotEnd)
        {
            // next available slot abuts released so extend new slot and remove previous next available slot
            slotEnd = next_slot_itr->first + next_slot_itr->second;
            removeAvailableSlot(next_slot_itr->first, next_slot_itr->second);
        }
    }
    else
    {
        auto prev_slot_itr = _offsetSizes.rbegin();
        size_t prev_slotEnd = prev_slot_itr->first + prev_slot_itr->second;
        if (prev_slotEnd == slotStart)
        {
            // previous slot abuts with the one being released so remove it.
            slotStart = prev_slot_itr->first;
            removeAvailableSlot(prev_slot_itr->first, prev_slot_itr->second);
        }
    }

    insertAvailableSlot(slotStart, slotEnd - slotStart);

    if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS) check();

    return true;
}
