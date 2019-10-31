#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <atomic>

namespace vsg
{

    template<typename T>
    void exchange_if_lower(std::atomic<T>& reference, T t)
    {
        T original_value = reference.load();
        while (t < original_value && !reference.compare_exchange_weak(original_value, t)) {}
    };

    template<typename T>
    void exchange_if_greater(std::atomic<T>& reference, T t)
    {
        T original_value = reference.load();
        while (t > original_value && !reference.compare_exchange_weak(original_value, t)) {}
    };

    template<typename T>
    void exchange_multiply(std::atomic<T>& reference, T t)
    {
        T original_value = reference.load();
        while (!reference.compare_exchange_weak(original_value, original_value * t)) {}
    };

    template<typename T>
    bool compare_exchange(std::atomic<T>& reference, T from, T to)
    {
        T original_value = from;
        return reference.compare_exchange_strong(original_value, to);
    };

} // namespace vsg
