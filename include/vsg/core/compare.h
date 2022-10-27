#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <cstring>

namespace vsg
{

    template<class T, class R>
    int compare_pointer(const ref_ptr<T>& lhs, const ref_ptr<R>& rhs)
    {
        if (lhs == rhs) return 0;
        return lhs ? (rhs ? lhs->compare(*rhs) : 1) : (rhs ? -1 : 0);
    }

    template<class T>
    int compare_value(const T& lhs, const T& rhs)
    {
        return (lhs < rhs) ? -1 : ((rhs < lhs) ? 1 : 0);
    }

    template<class T>
    int compare_values(const T* lhs, const T* rhs, size_t size)
    {
        int result = 0;
        for (size_t i = 0; i < size; ++i)
        {
            if ((result = compare_value(lhs[i], rhs[i]))) break;
        }
        return result;
    }

    template<typename T>
    int compare_memory(const T& lhs, const T& rhs)
    {
        return std::memcmp(&lhs, &rhs, sizeof(T));
    }

    template<typename S, typename E>
    int compare_region(const S& start, const E& end, const S& rhs)
    {
        const char* lhs_ptr = reinterpret_cast<const char*>(&start);
        const char* rhs_ptr = reinterpret_cast<const char*>(&rhs);
        size_t size = size_t(reinterpret_cast<const char*>(&end) - lhs_ptr) + sizeof(E);

        // use memcpy to compare the contents of the data
        return std::memcmp(lhs_ptr, rhs_ptr, size);
    }

    template<typename T>
    int compare_pointer_container(const T& lhs, const T& rhs)
    {
        if (lhs.size() < rhs.size()) return -1;
        if (lhs.size() > rhs.size()) return 1;
        if (lhs.empty()) return 0;

        auto rhs_itr = rhs.begin();
        for (auto& object : lhs)
        {
            int result = compare_pointer(object, *rhs_itr++);
            if (result != 0) return result;
        }
        return 0;
    }

    template<typename T>
    int compare_value_container(const T& lhs, const T& rhs)
    {
        if (lhs.size() < rhs.size()) return -1;
        if (lhs.size() > rhs.size()) return 1;
        if (lhs.empty()) return 0;

        return compare_region(lhs.front(), lhs.back(), rhs.front());
    }

    template<typename T>
    int compare_container(const T& lhs, const T& rhs)
    {
        if (lhs.size() < rhs.size()) return -1;
        if (lhs.size() > rhs.size()) return 1;
        if (lhs.empty()) return 0;

        auto rhs_itr = rhs.begin();
        for (auto& object : lhs)
        {
            int result = object.compare(*rhs_itr++);
            if (result != 0) return result;
        }
        return 0;
    }

    /// less functor for comparing ref_ptr<Object> typically used with std::set<> etc.
    struct DerefenceLess
    {
        template<class P>
        bool operator()(const P& lhs, const P& rhs) const
        {
            return lhs->compare(*rhs) < 0;
        }
    };

} // namespace vsg
