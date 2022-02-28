#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <map>
#include <list>
#include <ostream>

namespace vsg
{

    /// Mask for hint what checks/reporting to do when using MemorySlots within Allocator/Buffer/DeviceMemory.
    enum MemoryTracking
    {
        MEMORY_TRACKING_NO_CHECKS = 0,
        MEMORY_TRACKING_REPORT_ACTIONS = 1,
        MEMORY_TRACKING_CHECK_ACTIONS = 2,
        MEMORY_TRACKING_LOG_ACTIONS = 4,
#if 1
        MEMORY_TRACKING_DEFAULT = MEMORY_TRACKING_REPORT_ACTIONS | MEMORY_TRACKING_CHECK_ACTIONS
#else
        MEMORY_TRACKING_DEFAULT = MEMORY_TRACKING_NO_CHECKS
#endif
    };

    /** class used internally by vsg::Allocator, vsg::DeviceMemory and vsg::Buffer to manage allocation of within a block of CPU or GPU memory.*/
    class VSG_DECLSPEC MemorySlots
    {
    public:

        explicit MemorySlots(size_t availableMemorySize, int in_memoryTracking = MEMORY_TRACKING_DEFAULT);
        ~MemorySlots();

        using OptionalOffset = std::pair<bool, size_t>;
        OptionalOffset reserve(size_t size, size_t alignment);

        bool release(size_t offset, size_t size);

        bool full() const { return _availableMemory.empty(); }

        size_t maximumAvailableSpace() const { return _availableMemory.empty() ? 0 : _availableMemory.rbegin()->first; }
        size_t totalAvailableSize() const;
        size_t totalReservedSize() const;
        size_t totalMemorySize() const { return _totalMemorySize; }

        // debug factilities
        void report(std::ostream& out) const;
        bool check() const;

        struct Action
        {
            int action; // action==0 is MemorySlots::MemorySlots(availableMemorySize), action==1 is MemorySlots::reserve(size, alightment), action==2, MemorySlots::release(size, alightment)
            size_t offset;
            size_t size;
            size_t alignment;
        };

        mutable int memoryTracking = MEMORY_TRACKING_DEFAULT;
        std::list<Action> logOfActions;

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

} // namespace vsg
