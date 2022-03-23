#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/core/Inherit.h>
#include <vsg/io/stream.h>

#include <ostream>
#include <map>
#include <mutex>
#include <set>

namespace vsg
{

    /// class for facilitating the share of instances of objects that have the same properties.
    class VSG_DECLSPEC SharedObjects : public Inherit<Object, SharedObjects>
    {
    public:
        SharedObjects();

        template<class T>
        ref_ptr<T> shared_default()
        {
            std::scoped_lock<std::mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& def = _defaults[id];
            auto def_T = def.cast<T>(); // should be able to do a static cast
            if (!def_T)
            {
                def_T = share(T::create());
                def = def_T;
            }

            return def_T;
        }

        template<class T>
        ref_ptr<T> share(ref_ptr<T> object)
        {
            std::scoped_lock<std::mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& shared_objects = _sharedObjects[id];
            if (auto itr = shared_objects.find(object); itr != shared_objects.end())
            {
                return ref_ptr<T>(static_cast<T*>(itr->get()));
            }

            shared_objects.insert(object);
            return object;
        }

        /// clear all the internal structures leaving no Objects cached.
        void clear();

        /// write out stats of objects held, types of objects and their reference counts
        void report(std::ostream& out);

    protected:
        virtual ~SharedObjects();

        std::mutex _mutex;
        std::map<std::type_index, ref_ptr<Object>> _defaults;
        std::map<std::type_index, std::set<ref_ptr<Object>, DerefenceLess>> _sharedObjects;
    };
    VSG_type_name(vsg::SharedObjects);

} // namespace vsg
