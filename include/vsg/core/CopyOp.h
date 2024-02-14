#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <map>

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <vsg/io/Logger.h>

namespace vsg
{
    class CopyOp
    {
    public:
        std::map<const Object*, ref_ptr<Object>> duplicates;

        template<class T>
        ref_ptr<T> operator()(ref_ptr<T> ptr)
        {
            if (ptr)
            {
                if (auto itr = duplicates.find(ptr); itr != duplicates.end())
                {
                    if (!itr->second) itr->second = ptr->clone(*this);
                    if (itr->second) return itr->second.template cast<T>();

                    warn("Unable to clone ", ptr);
                }
            }
            return ptr;
        }

        /// container of pointers
        template<class C>
        C operator()(const C& src)
        {
            C dest;
            using T = typename C::value_type::element_type;
            dest.reserve(src.size());
            for (auto& ptr : src)
            {
                if (auto itr = duplicates.find(ptr); itr != duplicates.end())
                {
                    if (!itr->second) itr->second = ptr->clone(*this);
                    if (itr->second)
                    {
                        dest.push_back(itr->second.template cast<T>());
                        continue;
                    }
                    warn("Unable to clone ", ptr);
                }
                dest.push_back(ptr);
            }
            return dest;
        }

        void clear()
        {
            duplicates.clear();
        }

        void reset()
        {
            for (auto itr = duplicates.begin(); itr != duplicates.end(); ++itr)
            {
                itr->second.reset();
            }
        }
    };

} // namespace vsg
