/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#pragma once

#include <vsg/core/Export.h>

#include <set>
#include <thread>

namespace vsg
{

    struct Affinity
    {
        Affinity() {}

        explicit Affinity(uint32_t cpu, uint32_t num = 1)
        {
            for (uint32_t i = 0; i < num; ++i) cpus.insert(cpu + i);
        }

        std::set<uint32_t> cpus;

        operator bool() const { return !cpus.empty(); }
    };

    /// Set the CPU affinity of specified std::thread
    extern VSG_DECLSPEC void setAffinity(std::thread& thread, const Affinity& affinity);

    /// Set the CPU affinity of current thread
    /// Note, under Linux the CPU affinity of thread is inherited by any threads that it creates
    extern VSG_DECLSPEC void setAffinity(const Affinity& affinity);

} // namespace vsg
