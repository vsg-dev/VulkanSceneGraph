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
#include <ostream>
#include <vector>

namespace vsg
{

    /// Mask for hint what checks/reporting to do when using MemorySlots within Allocator/Buffer/DeviceMemory.
    enum MemoryTracking
    {
        MEMORY_TRACKING_NO_CHECKS = 0,
        MEMORY_TRACKING_REPORT_ACTIONS = 1,
        MEMORY_TRACKING_CHECK_ACTIONS = 2,
        MEMORY_TRACKING_DEFAULT = MEMORY_TRACKING_NO_CHECKS
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
        bool empty() const { return totalAvailableSize() == totalMemorySize(); }

        size_t maximumAvailableSpace() const { return _availableMemory.empty() ? 0 : _availableMemory.rbegin()->first; }
        size_t totalAvailableSize() const;
        size_t totalReservedSize() const;
        size_t totalMemorySize() const { return _totalMemorySize; }

        // debug facilities
        void report(std::ostream& out) const;
        bool check() const;

        mutable int memoryTracking = MEMORY_TRACKING_DEFAULT;

    protected:
        std::multimap<size_t, size_t> _availableMemory;
        std::map<size_t, size_t> _offsetSizes;
        std::map<size_t, size_t> _reservedMemory;

        void insertAvailableSlot(size_t offset, size_t size);
        void removeAvailableSlot(size_t offset, size_t size);

        size_t _totalMemorySize;
    };

} // namespace vsg
