#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <atomic>
#include <map>
#include <string>
#include <typeindex>
#include <vector>

#include <vsg/core/Export.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/type_name.h>

namespace vsg
{

    // forward declare
    class Auxiliary;
    class Visitor;
    class ConstVisitor;
    class RecordTraversal;
    class Input;
    class Output;
    class Object;
    class Duplicate;

    template<typename T>
    constexpr bool has_read_write() { return false; }

    class CopyOp
    {
    public:
        mutable ref_ptr<Duplicate> duplicate;

        /// copy/clone pointer
        template<class T>
        inline ref_ptr<T> operator()(ref_ptr<T> ptr) const;

        /// copy/clone container of pointers
        template<class C>
        inline C operator()(const C& src) const;

        explicit operator bool() const noexcept { return duplicate.valid(); }
    };

    VSG_type_name(vsg::Object);

    class VSG_DECLSPEC Object
    {
    public:
        Object();

        Object(const Object& object, const CopyOp& copyop = {});
        Object& operator=(const Object&);

        static ref_ptr<Object> create() { return ref_ptr<Object>(new Object); }

        static ref_ptr<Object> create_if(bool flag)
        {
            if (flag)
                return ref_ptr<Object>(new Object);
            else
                return {};
        }

        /// provide new and delete to enable custom memory management via the vsg::Allocator singleton, using the MEMORY_AFFINTY_OBJECTS
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr);

        virtual std::size_t sizeofObject() const noexcept { return sizeof(Object); }
        virtual const char* className() const noexcept { return type_name<Object>(); }

        /// return the std::type_info of this Object
        virtual const std::type_info& type_info() const noexcept { return typeid(Object); }
        virtual bool is_compatible(const std::type_info& type) const noexcept { return typeid(Object) == type; }

        template<class T>
        T* cast() { return is_compatible(typeid(T)) ? static_cast<T*>(this) : nullptr; }

        template<class T>
        const T* cast() const { return is_compatible(typeid(T)) ? static_cast<const T*>(this) : nullptr; }

        /// clone this object using CopyOp's duplicates map to decide whether to clone or to return the original object.
        /// The default clone(CopyOp&) implementation simply returns ref_ptr<> to this object rather attempt to clone.
        virtual ref_ptr<Object> clone(const CopyOp& copyop = {}) const;

        /// compare two objects, return -1 if this object is less than rhs, return 0 if it's equal, return 1 if rhs is greater,
        virtual int compare(const Object& rhs) const;

        virtual void accept(Visitor& visitor);
        virtual void traverse(Visitor&) {}

        virtual void accept(ConstVisitor& visitor) const;
        virtual void traverse(ConstVisitor&) const {}

        virtual void accept(RecordTraversal& visitor) const;
        virtual void traverse(RecordTraversal&) const {}

        virtual void read(Input& input);
        virtual void write(Output& output) const;

        // ref counting methods
        inline void ref() const noexcept { _referenceCount.fetch_add(1, std::memory_order_relaxed); }
        inline void unref() const noexcept
        {
            if (_referenceCount.fetch_sub(1, std::memory_order_seq_cst) <= 1) _attemptDelete();
        }
        inline void unref_nodelete() const noexcept { _referenceCount.fetch_sub(1, std::memory_order_seq_cst); }
        inline unsigned int referenceCount() const noexcept { return _referenceCount.load(); }

        /// meta data access methods
        /// wraps the value with a vsg::Value<T> object and then assigns via setObject(key, vsg::Value<T>)
        template<typename T>
        void setValue(const std::string& key, const T& value);

        /// specialization of setValue to handle passing C strings
        void setValue(const std::string& key, const char* value) { setValue(key, value ? std::string(value) : std::string()); }

        /// get specified value type, return false if value associated with key is not assigned or is not the correct type
        template<typename T>
        bool getValue(const std::string& key, T& value) const;

        /// assign an Object associated with key
        void setObject(const std::string& key, ref_ptr<Object> object);

        /// get Object pointer associated with key, return nullptr if no object associated with key has been assigned
        Object* getObject(const std::string& key);

        /// get const Object pointer associated with key, return nullptr if no object associated with key has been assigned
        const Object* getObject(const std::string& key) const;

        /// get object pointer of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        T* getObject(const std::string& key) { return dynamic_cast<T*>(getObject(key)); }

        /// get const object pointer of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        const T* getObject(const std::string& key) const { return dynamic_cast<const T*>(getObject(key)); }

        /// get ref_ptr<Object> associated with key, return nullptr if no object associated with key has been assigned
        ref_ptr<Object> getRefObject(const std::string& key);

        /// get ref_ptr<const Object> pointer associated with key, return nullptr if no object associated with key has been assigned
        ref_ptr<const Object> getRefObject(const std::string& key) const;

        /// get ref_ptr<T> of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        ref_ptr<T> getRefObject(const std::string& key) { return getRefObject(key).cast<T>(); }

        /// get ref_ptr<const T> of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        const ref_ptr<const T> getRefObject(const std::string& key) const { return getRefObject(key).cast<const T>(); }

        /// remove meta object or value associated with key
        void removeObject(const std::string& key);

        // Auxiliary object access methods, the optional Auxiliary is used to store meta data
        Auxiliary* getOrCreateAuxiliary();
        Auxiliary* getAuxiliary() { return _auxiliary; }
        const Auxiliary* getAuxiliary() const { return _auxiliary; }

    protected:
        virtual ~Object();

        virtual void _attemptDelete() const;
        void setAuxiliary(Auxiliary* auxiliary);

    private:
        friend class Auxiliary;

        mutable std::atomic_uint _referenceCount;

        Auxiliary* _auxiliary;
    };

    template<class T, class R>
    T* cast(const ref_ptr<R>& object)
    {
        return object ? object->template cast<T>() : nullptr;
    }

    template<class T, class R>
    T* cast(R* object)
    {
        return object ? object->template cast<T>() : nullptr;
    }

    template<>
    constexpr bool has_read_write<Object>() { return true; }

    using RefObjectPath = std::vector<ref_ptr<Object>>;
    using ObjectPath = std::vector<Object*>;

    class Duplicate : public Object
    {
    public:
        using DuplicateMap = std::map<const Object*, ref_ptr<Object>>;
        using iterator = DuplicateMap::iterator;
        using key_type = DuplicateMap::key_type;
        using mapped_type = DuplicateMap::mapped_type;

        DuplicateMap duplicates;

        inline iterator find(const key_type& key) { return duplicates.find(key); }
        inline iterator begin() { return duplicates.begin(); }
        inline iterator end() { return duplicates.end(); }
        std::size_t size() const { return duplicates.size(); }
        inline mapped_type& operator[](const Object* object) { return duplicates[object]; }

        bool contains(const Object* object) const { return duplicates.count(object) != 0; }
        void insert(const Object* first, ref_ptr<Object> second = {}) { duplicates[first] = second; }
        void clear() { duplicates.clear(); }

        void reset()
        {
            for (auto itr = duplicates.begin(); itr != duplicates.end(); ++itr)
            {
                itr->second.reset();
            }
        }
    };

    template<class T>
    inline ref_ptr<T> CopyOp::operator()(ref_ptr<T> ptr) const
    {
        if (ptr && duplicate)
        {
            if (auto itr = duplicate->find(ptr); itr != duplicate->end())
            {
                if (!itr->second) itr->second = ptr->clone(*this);
                if (itr->second) return itr->second.template cast<T>();
            }
        }
        return ptr;
    }

    template<class C>
    inline C CopyOp::operator()(const C& src) const
    {
        if (!duplicate) return src;

        C dest;
        dest.reserve(src.size());
        for (auto& ptr : src)
        {
            dest.push_back(operator()(ptr));
        }
        return dest;
    }

    template<class T>
    ref_ptr<T> clone(vsg::ref_ptr<const T> object)
    {
        if (!object) return {};
        auto new_object = object->clone();
        return new_object.template cast<T>();
    }

    template<class T>
    ref_ptr<T> clone(vsg::ref_ptr<T> object)
    {
        if (!object) return {};
        auto new_object = object->clone();
        return new_object.template cast<T>();
    }

} // namespace vsg
