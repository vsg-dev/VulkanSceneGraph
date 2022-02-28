/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Exception.h>

#include <algorithm>
#include <iostream>

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
        std::cout<<"MemorySlots::MemorySlots("<<availableMemorySize<<") "<<this<<std::endl;
    }
    if (memoryTracking & MEMORY_TRACKING_LOG_ACTIONS)
    {
        logOfActions.push_back(Action{0, 0, availableMemorySize, 0});
    }

    _availableMemory.insert(SizeOffset(availableMemorySize, 0));
    _offsetSizes.insert(OffsetSize(0, availableMemorySize));

    _totalMemorySize = availableMemorySize;

}

MemorySlots::~MemorySlots()
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        if (_availableMemory.size()==1)
        {
            std::cout<<"MemorySlots::~MemorySlots() "<<this<<", all slots restored correctly."<<std::endl;
        }
        else
        {
            std::cout<<"MemorySlots::~MemorySlots() "<<this<<", not all slots restored correctly."<<std::endl;
            report(std::cout);
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

        report(std::cout);

        // throw Exception{"MemorySlots check failed", 0};

        return false;
    }

    return true;
}

void MemorySlots::report(std::ostream& out) const
{
    out << "MemorySlots::report() " <<this<< std::endl;
    for (auto& [offset, size] : _offsetSizes)
    {
        out << "    available " << offset << ", " << size << std::endl;
    }

    for (auto& [offset, size] : _reservedOffsetSizes)
    {
         out << "    reserved " << std::dec << offset << ", " << size << std::endl;
    }

    if (!logOfActions.empty())
    {
        out<<"MemorySlots::reportActions() "<<this<<" number of actions "<<logOfActions.size()<<std::endl;
        for(auto& act : logOfActions)
        {
            out<<"   action = "<<act.action<<", offset = "<<act.offset<<", size = "<<act.size<<", alignment = "<<act.alignment<<std::endl;
        }
    }
}

MemorySlots::OptionalOffset MemorySlots::reserve(size_t size, size_t alignment)
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout<<"MemorySlots::reserve("<<size<<", "<<alignment<<") "<<this<<std::endl;
    }

    if (memoryTracking & MEMORY_TRACKING_LOG_ACTIONS)
    {
        logOfActions.push_back(Action{1, 0, size, alignment});
    }

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

            if (memoryTracking & MEMORY_TRACKING_LOG_ACTIONS)
            {
                logOfActions.push_back(Action{2, alignedStart, size, alignment});
            }

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

bool MemorySlots::release(size_t offset, size_t size)
{
    if (memoryTracking & MEMORY_TRACKING_REPORT_ACTIONS)
    {
        std::cout<<"MemorySlots::release("<<offset<<", "<<size<<") "<<this<<std::endl;
    }

    if (memoryTracking & MEMORY_TRACKING_LOG_ACTIONS)
    {
        logOfActions.push_back(Action{3, offset, size, 0});
    }

    auto reserved_itr = _reservedOffsetSizes.find(offset);
    if (reserved_itr == _reservedOffsetSizes.end())
    {
        if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS)
        {
            std::cout << "MemorySlots::release("<<offset<<", "<<size<<") "<<this<<", slot not found A" << std::endl;

            report(std::cout);

            // throw Exception{"MemorySlots::release() slot found A", 0};
        }
        return false;
    }
    else
    {
        if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS)
        {
            if (reserved_itr->second != size)
            {
                std::cout << "MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second = " << std::dec << reserved_itr->second << ", size=" << size << std::endl;
                report(std::cout);
                //if (size != 0) throw Exception{"MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second",0};
            }
            else
            {
                std::cout << "    MemorySlots::release("<<offset<<", "<<size<<") "<<this<<", slot found B" << std::endl;
                report(std::cout);
                return false;
            }
        }

        size = reserved_itr->second;

        _reservedOffsetSizes.erase(reserved_itr);
    }

    if (_offsetSizes.empty())
    {
        // first empty space
        _availableMemory.insert(SizeOffset(size, offset));
        _offsetSizes.insert(OffsetSize(offset, size));

        if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS) check();

        return true;
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

    if (memoryTracking & MEMORY_TRACKING_CHECK_ACTIONS)
    {
        check();
    }

    return true;
}
