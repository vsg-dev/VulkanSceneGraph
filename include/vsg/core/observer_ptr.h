#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Auxiliary.h>

namespace vsg
{

    template<class T>
    class observer_ptr
    {
    public:

        observer_ptr() {}

        observer_ptr(const observer_ptr& rhs) :
            _auxiliary(rhs._auxiliary)
        {
        }

        template<class R>
        observer_ptr(R* ptr):
            _auxiliary(ptr ? ptr->getOrCreateUniqueAuxiliary() : nullptr)
        {
        }

        template<class R>
        observer_ptr(const observer_ptr<R>& ptr):
            _auxiliary(ptr._auxiliary)
        {
        }

        template<class R>
        observer_ptr(const ref_ptr<R>& ptr):
            _auxiliary(ptr.valid() ? ptr->getOrCreateUniqueAuxiliary() : nullptr)
        {
        }

        ~observer_ptr()
        {
        }

        template<class R>
        observer_ptr& operator = (R* ptr)
        {
            _auxiliary = ptr ? ptr->getOrCreateUniqueAuxiliary() : nullptr;
            return *this;
        }

        observer_ptr& operator = (const observer_ptr& rhs)
        {
            _auxiliary = rhs._auxiliary;
            return *this;
        }

        template<class R>
        observer_ptr& operator = (const observer_ptr<R>& rhs)
        {
            _auxiliary = rhs._auxiliary;
            return *this;
        }

        template<class R>
        observer_ptr& operator = (const ref_ptr<R>& rhs)
        {
            _auxiliary = rhs.valid() ? rhs->getOrCreateUniqueAuxiliary() : nullptr;
            return *this;
        }

        bool valid() const { return _auxiliary.valid() && _auxiliary->getConnectedObject()!=nullptr; }

        explicit operator bool() const { return valid(); }

        /// convert observer_ptr into a ref_ptr so that Object that pointed to can be safely accessed.
        template<class R>
        operator ref_ptr<R> () const
        {
            if (!_auxiliary.valid()) return ref_ptr<R>();
            return static_cast<T*>(_auxiliary->getConnectedObject());
        }

    protected:

        template<class R> friend class observer_ptr;

        ref_ptr<Auxiliary> _auxiliary;

    };

}
