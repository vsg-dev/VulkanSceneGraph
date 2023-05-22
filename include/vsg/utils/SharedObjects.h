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

    // forward declare
    class SuitableForSharing;

    /// class for facilitating the share of instances of objects that have the same properties.
    class VSG_DECLSPEC SharedObjects : public Inherit<Object, SharedObjects>
    {
    public:
        SharedObjects();

        template<class T>
        ref_ptr<T> shared_default();

        template<class T>
        void share(ref_ptr<T>& object);

        template<class T, typename Func>
        void share(ref_ptr<T>& object, Func init);

        template<class C>
        void share(C& container);

        /// visitor that checks a loaded object, and it's children whether it is suitable for sharing in SharedObjects
        ref_ptr<SuitableForSharing> suitableForSharing;

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
        std::map<std::type_index, std::set<ref_ptr<Object>, DereferenceLess>> _sharedObjects;
    };
    VSG_type_name(vsg::SharedObjects);

    /// Helper class for support sharing of objects loaded from files.
    class VSG_DECLSPEC LoadedObject : public Inherit<Object, LoadedObject>
    {
    public:
        Path filename;
        ref_ptr<Options> options;
        ref_ptr<Object> object;

        LoadedObject(const Path& in_filename, ref_ptr<const Options> in_options, ref_ptr<Object> in_object = {});

        void traverse(Visitor& visitor) override;
        void traverse(ConstVisitor& visitor) const override;

        int compare(const Object& rhs_object) const override;
    };
    VSG_type_name(vsg::LoadedObject);

    /// Helper class for deciding whether sharing is permitted for this type - required to avoid circular references
    /// If an object that is put forward to be shared via the SharedObjects container then it can't contain any references to vsg::Options & associated SharedOjbjects
    /// otherwise a circular reference can be created that prevents all the objects in the circular reference chain from being deleted. Such objects must
    /// be prevent from inclusion in the SharedObejcts. An example of class not suitable is vsg::PagedLOD as it has an options member. If your subclasses are
    /// like PagedLOD and might create a circular reference when using with SharedOjbects then you should subclass from SuitableForSharing and add support
    /// for your class and when instances are found set the suitableForSharing flag to false, and then assign this to the SharedOjbects container so that
    /// it can make sure it's not treated as suitable for inclusions.
    class VSG_DECLSPEC SuitableForSharing : public Inherit<ConstVisitor, SuitableForSharing>
    {
    public:
        bool suitableForSharing = true;

        void apply(const Object& object) override;
        void apply(const PagedLOD& plod) override;

        bool suitable(const Object* object)
        {
            suitableForSharing = true;
            if (object) object->accept(*this);
            return suitableForSharing;
        }
    };
    VSG_type_name(vsg::SuitableForSharing);

    // implementation of template method
    template<class T>
    ref_ptr<T> SharedObjects::shared_default()
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

    // implementation of template method
    template<class T>
    void SharedObjects::share(ref_ptr<T>& object)
    {
        std::scoped_lock<std::recursive_mutex> lock(_mutex);

        if (suitableForSharing && !suitableForSharing->suitable(object.get())) return;

        auto id = std::type_index(typeid(T));
        auto& shared_objects = _sharedObjects[id];
        if (auto itr = shared_objects.find(object); itr != shared_objects.end())
        {
            object = ref_ptr<T>(static_cast<T*>(itr->get()));
            return;
        }

        shared_objects.insert(object);
    }

    // implementation of template method
    template<class T, typename Func>
    void SharedObjects::share(ref_ptr<T>& object, Func init)
    {
        {
            std::scoped_lock<std::recursive_mutex> lock(_mutex);

            auto id = std::type_index(typeid(T));
            auto& shared_objects = _sharedObjects[id];
            if (auto itr = shared_objects.find(object); itr != shared_objects.end())
            {
                object = ref_ptr<T>(static_cast<T*>(itr->get()));
                return;
            }
        }

        init(object);

        {
            std::scoped_lock<std::recursive_mutex> lock(_mutex);
            auto id = std::type_index(typeid(T));
            auto& shared_objects = _sharedObjects[id];
            if (suitableForSharing && suitableForSharing->suitable(object.get()))
            {
                shared_objects.insert(object);
            }
        }
    }

    // implementation of template method
    template<class C>
    void SharedObjects::share(C& container)
    {
        for (auto& object : container)
        {
            share(object);
        }
    }

} // namespace vsg
