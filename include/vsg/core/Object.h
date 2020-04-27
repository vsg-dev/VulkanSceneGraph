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

namespace vsg
{

    // forward declare
    class Auxiliary;
    class Visitor;
    class ConstVisitor;
    class RecordTraversal;
    class CullTraversal;
    class Allocator;
    class Input;
    class Output;

    template<typename T>
    constexpr bool has_read_write() { return false; }

    class VSG_DECLSPEC Object
    {
    public:
        Object();

        explicit Object(Allocator* allocator);

        Object(const Object&);
        Object& operator=(const Object&);

        //static ref_ptr<Object> create(Allocator* allocator=nullptr);

        virtual std::size_t sizeofObject() const noexcept { return sizeof(Object); }
        virtual const char* className() const noexcept { return "vsg::Object"; }

        virtual void accept(Visitor& visitor);
        virtual void traverse(Visitor&) {}

        virtual void accept(ConstVisitor& visitor) const;
        virtual void traverse(ConstVisitor&) const {}

        virtual void accept(RecordTraversal& visitor) const;
        virtual void traverse(RecordTraversal&) const {}

        virtual void accept(CullTraversal& visitor) const;
        virtual void traverse(CullTraversal&) const {}

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

        // meta data access methods
        template<typename T>
        void setValue(const std::string& key, const T& value);
        void setValue(const std::string& key, const char* value) { setValue(key, value ? std::string(value) : std::string()); }

        template<typename T>
        bool getValue(const std::string& key, T& value) const;

        void setObject(const std::string& key, Object* object);
        Object* getObject(const std::string& key);
        const Object* getObject(const std::string& key) const;

        template<class T>
        T* getObject(const std::string& key) { return dynamic_cast<T*>(getObject(key)); }

        template<class T>
        const T* getObject(const std::string& key) const{ return dynamic_cast<const T*>(getObject(key)); }

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

    template<>
    constexpr bool has_read_write<Object>() { return true; }

} // namespace vsg
