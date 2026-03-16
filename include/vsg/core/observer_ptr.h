#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

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
            _ptr(nullptr),
            _refCount(nullptr)
        {}

        observer_ptr(const observer_ptr& rhs) :
            _ptr(nullptr),
            _refCount(rhs._refCount)
        {
            if (_refCount)
            {
                _ptr = rhs._ptr;
                _refCount->incrementObservers();
            }
        }

        template<class R>
        explicit observer_ptr(R* ptr) :
            _ptr(ptr),
            _refCount(ptr ? ptr->_referenceCount : nullptr)
        {
            if (_ptr)
                _refCount->incrementObservers();
        }

        template<class R>
        explicit observer_ptr(const observer_ptr<R>& ptr) :
            _ptr(nullptr),
            _refCount(ptr._refCount)
        {
            if (_refCount)
            {
                _ptr = ptr._ptr;
                _refCount->incrementObservers();
            }
        }

        template<class R>
        explicit observer_ptr(const vsg::ref_ptr<R>& ptr) :
            _ptr(ptr.get()),
            _refCount(ptr.valid() ? ptr->_referenceCount : nullptr)
        {
            if (_refCount)
                _refCount->incrementObservers();
        }

        ~observer_ptr()
        {
            if (_refCount)
                _refCount->decrementObservers();
        }

        void reset()
        {
            _ptr = nullptr;
            if (_refCount)
                _refCount->decrementObservers();
            _refCount = nullptr;
        }

        template<class R>
        observer_ptr& operator=(R* ptr)
        {
            if (_refCount)
                _refCount->decrementObservers();
            _ptr = ptr;
            _refCount = ptr ? ptr->_referenceCount : nullptr;
            if (_refCount)
                _refCount->incrementObservers();
            return *this;
        }

        observer_ptr& operator=(const observer_ptr& rhs)
        {
            if (_refCount)
                _refCount->decrementObservers();
            _ptr = rhs._ptr;
            _refCount = rhs._refCount;
            if (_refCount)
                _refCount->incrementObservers();
            return *this;
        }

        template<class R>
        observer_ptr& operator=(const observer_ptr<R>& rhs)
        {
            if (_refCount)
                _refCount->decrementObservers();
            _ptr = rhs._ptr;
            _refCount = rhs._refCount;
            if (_refCount)
                _refCount->incrementObservers();
            return *this;
        }

        template<class R>
        observer_ptr& operator=(const vsg::ref_ptr<R>& rhs)
        {
            if (_refCount)
                _refCount->decrementObservers();
            _ptr = rhs.get();
            _refCount = rhs.valid() ? rhs->_referenceCount : nullptr;
            if (_refCount)
                _refCount->incrementObservers();
            return *this;
        }

        template<class R>
        bool operator<(const observer_ptr<R>& rhs) const { return (rhs._ptr < _ptr) || (rhs._ptr == _ptr && rhs._refCount < _refCount); }

        template<class R>
        bool operator==(const observer_ptr<R>& rhs) const { return (rhs._refCount == _refCount); }

        template<class R>
        bool operator!=(const observer_ptr<R>& rhs) const { return (rhs._refCount != _refCount); }

        template<class R>
        bool operator<(const R* rhs) const
        {
            if (rhs < _ptr)
                return true;
            if (_ptr == nullptr)
                return false;
            return rhs->_referenceCount < _refCount;
        }

        template<class R>
        bool operator==(const R* rhs) const
        {
            if (rhs != _ptr)
                return false;
            if (rhs == nullptr)
                return true;
            return rhs->_referenceCount == _refCount;
        }

        template<class R>
        bool operator!=(const R* rhs) const
        {
            if (rhs != _ptr)
                return true;
            if (rhs == nullptr)
                return false;
            return rhs->_referenceCount != _refCount;
        }

        template<class R>
        bool operator<(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() < _ptr)
                return true;
            if (_ptr == nullptr)
                return false;
            return rhs->_referenceCount < _refCount;
        }

        template<class R>
        bool operator==(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() != _ptr)
                return false;
            if (rhs == nullptr)
                return true;
            return rhs->_referenceCount == _refCount;
        }

        template<class R>
        bool operator!=(const vsg::ref_ptr<R>& rhs) const
        {
            if (rhs.get() != _ptr)
                return true;
            if (rhs == nullptr)
                return false;
            return rhs->_referenceCount != _refCount;
        }

        bool valid() const noexcept { return _refCount && _refCount->useCount() > 0; }

        explicit operator bool() const noexcept { return valid(); }

        /// convert observer_ptr into a ref_ptr so that Object pointed to can be safely accessed.
        vsg::ref_ptr<T> ref_ptr() const { return vsg::ref_ptr<T>(*this); }

        /// convert observer_ptr into a ref_ptr so that Object pointed to can be safely accessed.
        template<class R>
        operator vsg::ref_ptr<R>() const
        {
            if (_refCount && _refCount->incrementIfNonZero())
            {
                detail::TemporaryOwner<R> temp{_ptr};
                return vsg::ref_ptr<R>(temp);
            }
            else
            {
                return {};
            }
        }

    protected:
        template<class R>
        friend class observer_ptr;

        T* _ptr;
        RefCountBase* _refCount;
    };

} // namespace vsg
