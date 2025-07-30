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

    /// weak smart pointer that works in conjunction with vsg::ref_ptr<> and vsg::Object/vsg::Auxiliary to
    /// provide broadly similar functionality to std::weak_ptr<>.
    template<class T>
    class observer_ptr
    {
    public:
        using element_type = T;

        observer_ptr() :
            _ptr(nullptr) {}

        observer_ptr(const observer_ptr& rhs) :
            _ptr(rhs._ptr),
            _auxiliary(rhs._auxiliary)
        {
        }

        template<class R>
        explicit observer_ptr(R* ptr) :
            _ptr(ptr),
            _auxiliary(ptr ? ptr->getOrCreateAuxiliary() : nullptr)
        {
        }

        template<class R>
        explicit observer_ptr(const observer_ptr<R>& ptr) :
            _ptr(ptr._ptr),
            _auxiliary(ptr._auxiliary)
        {
        }

        template<class R>
        explicit observer_ptr(const vsg::ref_ptr<R>& ptr) :
            _ptr(ptr.get()),
            _auxiliary(ptr.valid() ? ptr->getOrCreateAuxiliary() : nullptr)
        {
        }

        ~observer_ptr()
        {
        }

        void reset()
        {
            _ptr = nullptr;
            _auxiliary.reset();
        }

        template<class R>
        observer_ptr& operator=(R* ptr)
        {
            _ptr = ptr;
            _auxiliary = ptr ? ptr->getOrCreateAuxiliary() : nullptr;
            return *this;
        }

        observer_ptr& operator=(const observer_ptr& rhs)
        {
            _ptr = rhs._ptr;
            _auxiliary = rhs._auxiliary;
            return *this;
        }

        template<class R>
        observer_ptr& operator=(const observer_ptr<R>& rhs)
        {
            _ptr = rhs._ptr;
            _auxiliary = rhs._auxiliary;
            return *this;
        }

        template<class R>
        observer_ptr& operator=(const vsg::ref_ptr<R>& rhs)
        {
            _ptr = rhs.get();
            _auxiliary = rhs.valid() ? rhs->getOrCreateAuxiliary() : nullptr;
            return *this;
        }

        template<class R>
        bool operator<(const observer_ptr<R>& rhs) const { return (rhs._ptr < _ptr) || (rhs._ptr == _ptr && rhs._auxiliary < _auxiliary); }

        template<class R>
        bool operator==(const observer_ptr<R>& rhs) const { return (rhs._auxiliary == _auxiliary); }

        template<class R>
        bool operator!=(const observer_ptr<R>& rhs) const { return (rhs._auxiliary != _auxiliary); }

        template<class R>
        bool operator<(const R* rhs) const
        {
            if (rhs < _ptr)
                return true;
            if (_ptr == nullptr)
                return false;
            return rhs->getAuxiliary() < _auxiliary;
        }

        template<class R>
        bool operator==(const R* rhs) const
        {
            if (rhs != _ptr)
                return false;
            if (rhs == nullptr)
                return true;
            return rhs->getAuxiliary() == _auxiliary;
        }

        template<class R>
        bool operator!=(const R* rhs) const
        {
            if (rhs != _ptr)
                return true;
            if (rhs == nullptr)
                return false;
            return rhs->getAuxiliary() != _auxiliary;
        }

        template<class R>
        bool operator<(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() < _ptr)
                return true;
            if (_ptr == nullptr)
                return false;
            return rhs->getAuxiliary() < _auxiliary;
        }

        template<class R>
        bool operator==(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() != _ptr)
                return false;
            if (rhs == nullptr)
                return true;
            return rhs->getAuxiliary() == _auxiliary;
        }

        template<class R>
        bool operator!=(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() != _ptr)
                return true;
            if (rhs == nullptr)
                return false;
            return rhs->getAuxiliary() != _auxiliary;
        }

        bool valid() const noexcept { return _auxiliary.valid() && _auxiliary->getConnectedObject() != nullptr; }

        explicit operator bool() const noexcept { return valid(); }

        /// convert observer_ptr into a ref_ptr so that Object pointed to can be safely accessed.
        vsg::ref_ptr<T> ref_ptr() const { return vsg::ref_ptr<T>(*this); }

        /// convert observer_ptr into a ref_ptr so that Object pointed to can be safely accessed.
        template<class R>
        operator vsg::ref_ptr<R>() const
        {
            if (!_auxiliary) return vsg::ref_ptr<R>();

            std::scoped_lock<std::mutex> guard(_auxiliary->getMutex());
            if (_auxiliary->getConnectedObject() != nullptr)
                return vsg::ref_ptr<R>(_ptr);
            else
                return {};
        }

    protected:
        template<class R>
        friend class observer_ptr;

        T* _ptr;
        vsg::ref_ptr<Auxiliary> _auxiliary;
    };

} // namespace vsg
