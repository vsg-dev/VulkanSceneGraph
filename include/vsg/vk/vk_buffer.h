#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Version.h>

#include <array>
#include <vector>

namespace vsg
{

#if VSG_MAX_DEVICES <= 1

    /// vk_buffer that manages a single logical device support.
    template<class T>
    struct vk_buffer
    {
        T value;

        T& operator[](uint32_t) { return value; }
        const T& operator[](uint32_t) const { return value; }

        uint32_t size() const { return 1; }

        void clear() { value = {}; }

        auto begin() { return &value; }
        auto begin() const { return &value; }

        auto end() { return &value + 1; }
        auto end() const { return &value + 1; }
    };

#elif VSG_MAX_DEVICES <= 4

    /// vk_buffer that manages a fixed maximum number of logical Devices supported.
    template<class T>
    struct vk_buffer
    {
        std::array<T, VSG_MAX_DEVICES> buffer;

        T& operator[](uint32_t deviceID) { return buffer[deviceID]; }
        const T& operator[](uint32_t deviceID) const { return buffer[deviceID]; }

        uint32_t size() const { return VSG_MAX_DEVICES; }

        void clear()
        {
            for (auto& value : buffer) value = {};
        }

        auto begin() { return buffer.begin(); }
        auto begin() const { return buffer.begin(); }

        auto end() { return buffer.end(); }
        auto end() const { return buffer.end(); }
    };

#else

    /// vk_buffer that manages a variable number of logical Devices supported.
    template<class T>
    struct vk_buffer
    {
        std::vector<T> buffer;

        T& operator[](uint32_t deviceID)
        {
            if (deviceID >= buffer.size()) buffer.resize(deviceID + 1);
            return buffer[deviceID];
        }

        const T& operator[](uint32_t deviceID) const
        {
            return buffer[deviceID];
        }

        uint32_t size() const { return static_cast<uint32_t>(buffer.size()); }

        void clear() { buffer.clear(); }

        auto begin() { return buffer.begin(); }
        auto begin() const { return buffer.begin(); }

        auto end() { return buffer.end(); }
        auto end() const { return buffer.end(); }
    };

#endif

} // namespace vsg
