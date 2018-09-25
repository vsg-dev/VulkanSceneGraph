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

namespace vsg
{

    // forward declare
    class Auxiliary;
    class Visitor;

    class VSG_EXPORT Object
    {
    public:
        Object();

        template<class V> void t_traverse(V& visitor) {}

        virtual void accept(Visitor& visitor);
        virtual void traverse(Visitor&) {}

        inline void ref() const noexcept { _referenceCount.fetch_add(1, std::memory_order_relaxed); }
        inline void unref() const noexcept { if (_referenceCount.fetch_sub(1, std::memory_order_seq_cst)<=1) _delete(); }
        inline void unref_nodelete() const noexcept { _referenceCount.fetch_sub(1, std::memory_order_seq_cst); }
        inline unsigned int referenceCount() const noexcept { return _referenceCount.load(); }

        struct Key
        {
            Key(const char* in_name) : name(in_name), index(0) {}
            Key(const std::string& in_name) : name(in_name), index(0) {}
            Key(int in_index) : index(in_index) {}

            Key(const char* in_name, int in_index) : name(in_name), index(in_index) {}
            Key(const std::string& in_name, int in_index) : name(in_name), index(in_index) {}

            bool operator < (const Key& rhs) const
            {
                if (name<rhs.name) return true;
                if (name>rhs.name) return false;
                return (index<rhs.index);
            }

            std::string name;
            int         index;
        };

        template<typename T>
        void setValue(const Key& key, const T& value);

        void setValue(const Key& key, const char* value) { setValue(key, value ? std::string(value) : std::string()); }

        template<typename T>
        bool getValue(const Key& key, T& value) const;

        void setObject(const Key& key, Object* object);
        Object* getObject(const Key& key);
        const Object* getObject(const Key& key) const;

        Auxiliary* getOrCreateAuxiliary();
        Auxiliary* getAuxiliary() { return _auxiliary; }
        const Auxiliary* getAuxiliary() const { return _auxiliary; }

    protected:
        virtual ~Object();

        virtual void _delete() const;

        mutable std::atomic_uint _referenceCount;

        Auxiliary* _auxiliary;
    };

}
