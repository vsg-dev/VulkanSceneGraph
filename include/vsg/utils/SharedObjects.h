#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/compare.h>
#include <vsg/io/stream.h>

#include <map>
#include <mutex>
#include <ostream>
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
            std::scoped_lock<std::recursive_mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& def = _defaults[id];
            auto def_T = def.cast<T>(); // should be able to do a static cast
            if (!def_T)
            {
                def_T = T::create();
                auto& shared_objects = _sharedObjects[id];
                if (auto itr = shared_objects.find(def_T); itr != shared_objects.end())
                {
                    def_T = (static_cast<T*>(itr->get()));
                }
                else
                {
                    shared_objects.insert(def_T);
                }

                def = def_T;
            }

            return def_T;
        }

        template<class T>
        void share(ref_ptr<T>& object)
        {
            std::scoped_lock<std::recursive_mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& shared_objects = _sharedObjects[id];
            if (auto itr = shared_objects.find(object); itr != shared_objects.end())
            {
                object = ref_ptr<T>(static_cast<T*>(itr->get()));
                return;
            }

            shared_objects.insert(object);
        }

        template<class T, typename Func>
        void share(ref_ptr<T>& object, Func init)
        {
            std::scoped_lock<std::recursive_mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& shared_objects = _sharedObjects[id];
            if (auto itr = shared_objects.find(object); itr != shared_objects.end())
            {
                object = ref_ptr<T>(static_cast<T*>(itr->get()));
                return;
            }

            init(object);

            shared_objects.insert(object);
        }

        template<class C>
        void share(C& container)
        {
            for (auto& object : container)
            {
                share(object);
            }
        }

        /// set of lower case file extensions for file types that should not be included in this SharedObjects
        std::set<Path> excludedExtensions;

        /// return true if the filename is of a type suitable for inclusion this SharedObjects
        virtual bool suitable(const Path& filename) const;

        /// check for an entry associated with filename.
        virtual bool contains(const Path& filename, ref_ptr<const Options> options = {}) const;

        /// add entry that matches filename and option.
        virtual void add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options = {});

        /// remove entry associated with filename.
        virtual bool remove(const Path& filename, ref_ptr<const Options> options = {});

        /// clear all the internal structures leaving no Objects cached.
        void clear();

        // clear all the singly referenced objects
        void prune();

        /// write out stats of objects held, types of objects and their reference counts
        void report(std::ostream& out);

    protected:
        virtual ~SharedObjects();

        mutable std::recursive_mutex _mutex;
        std::map<std::type_index, ref_ptr<Object>> _defaults;
        std::map<std::type_index, std::set<ref_ptr<Object>, DerefenceLess>> _sharedObjects;
    };
    VSG_type_name(vsg::SharedObjects);

    /// Helper class for support sharing of objects loaded from files.
    class LoadedObject : public Inherit<Object, LoadedObject>
    {
    public:
        Path filename;
        ref_ptr<const Options> options;
        ref_ptr<Object> object;

        LoadedObject(const Path& in_filename, ref_ptr<const Options> in_options, ref_ptr<Object> in_object = {}) :
            filename(in_filename),
            options(in_options),
            object(in_object) {}

        int compare(const Object& rhs_object) const override
        {
            int result = Object::compare(rhs_object);
            if (result != 0) return result;

            auto& rhs = static_cast<decltype(*this)>(rhs_object);

            if ((result = filename.compare(rhs.filename))) return result;
            return compare_pointer(options, rhs.options);
        }
    };
    VSG_type_name(vsg::LoadedObject);

} // namespace vsg
