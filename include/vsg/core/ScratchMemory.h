#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

#include <algorithm>

namespace vsg
{
    /** Lightweight allocator for temporary memory such as C structures allocated for Vulkan calls that don't require destruction.*/
    struct ScratchMemory : public Inherit<Object, ScratchMemory>
    {
        uint8_t* buffer = nullptr;
        uint8_t* ptr = nullptr;
        size_t size = 0;

        ref_ptr<ScratchMemory> next;

        explicit ScratchMemory(size_t bufferSize)
        {
            size = bufferSize;
            buffer = new uint8_t[bufferSize];
            ptr = buffer;
        }

        ScratchMemory(const ScratchMemory&) = delete;
        ScratchMemory& operator=(const ScratchMemory&) = delete;

        ~ScratchMemory()
        {
            delete[] buffer;
        }

        uint8_t* align(uint8_t* p) const
        {
            auto alignment = sizeof(p);
            uint8_t* new_p = reinterpret_cast<uint8_t*>(((reinterpret_cast<size_t>(p) + alignment - 1) / alignment) * alignment);
            return new_p;
        }

        template<typename T>
        T* allocate(size_t num = 1)
        {
            if (num == 0) return nullptr;

            size_t allocate_size = sizeof(T) * num;

            if ((ptr + allocate_size) <= (buffer + size))
            {
                T* allocated_ptr = reinterpret_cast<T*>(ptr);

                // move the ptr to the alignment end
                ptr = align(ptr + allocate_size);

                return allocated_ptr;
            }

            if (!next) next = ScratchMemory::create(std::max(size, allocate_size));

            return next->allocate<T>(num);
        }

        void release()
        {
            ptr = buffer;
            if (next) next->release();
        }
    };

} // namespace vsg
