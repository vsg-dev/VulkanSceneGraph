#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <atomic>
#include <string>

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
    class Allocator;
    class Input;
    class Output;
    class Object;

    template<typename T>
    constexpr bool has_read_write() { return false; }

    VSG_type_name(vsg::Object);

    class VSG_DECLSPEC Object
    {
    public:
        Object();

        explicit Object(Allocator* allocator);

        Object(const Object&);
        Object& operator=(const Object&);

        //static ref_ptr<Object> create(Allocator* allocator=nullptr);

        virtual std::size_t sizeofObject() const noexcept { return sizeof(Object); }
        virtual const char* className() const noexcept { return type_name<Object>(); }

        /// return the std::type_info of this Object
        virtual const std::type_info& type_info() const noexcept { return typeid(Object); }
        virtual bool is_compatible(const std::type_info& type) const noexcept { return typeid(Object) == type; }

        template<class T>
        T* cast() { return is_compatible(typeid(T)) ? static_cast<T*>(this) : nullptr; }

        template<class T>
        const T* cast() const { return is_compatible(typeid(T)) ? static_cast<const T*>(this) : nullptr; }

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

        /// specialization of setValue to handle passing c strings
        void setValue(const std::string& key, const char* value) { setValue(key, value ? std::string(value) : std::string()); }

        /// get specified value type, return false if value associated with key is not assigned or is not the correct type
        template<typename T>
        bool getValue(const std::string& key, T& value) const;

        /// assign an Object associated with key
        void setObject(const std::string& key, Object* object);

        /// get Object associated with key, return nullptr if no object associated with key has been assigned
        Object* getObject(const std::string& key);

        /// get const Object associated with key, return nullptr if no object associated with key has been assigned
        const Object* getObject(const std::string& key) const;

        /// get object of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        T* getObject(const std::string& key) { return dynamic_cast<T*>(getObject(key)); }

        /// get const object of specified type associated with key, return nullptr if no object associated with key has been assigned
        template<class T>
        const T* getObject(const std::string& key) const { return dynamic_cast<const T*>(getObject(key)); }

        /// remove meta object or value associated with key
        void removeObject(const std::string& key);

        // Auxiliary object access methods, the optional Auxiliary is used to store meta data and links to Allocator
        Auxiliary* getOrCreateUniqueAuxiliary();
        Auxiliary* getAuxiliary() { return _auxiliary; }
        const Auxiliary* getAuxiliary() const { return _auxiliary; }

        // convenience method for getting the optional Allocator, if present this Allocator would have been used to create this Objects memory
        Allocator* getAllocator() const;

    protected:
        virtual ~Object();

        virtual void _attemptDelete() const;
        void setAuxiliary(Auxiliary* auxiliary);

    private:
        friend class Allocator;
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

} // namespace vsg
