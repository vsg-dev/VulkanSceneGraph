#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

namespace vsg
{

    template<class T>
    class ref_ptr
    {
    public:
        ref_ptr() noexcept :
            _ptr(nullptr) {}

        ref_ptr(const ref_ptr& rhs) noexcept :
            _ptr(rhs._ptr)
        {
            if (_ptr) _ptr->ref();
        }

        /// move constructor
        template<class R>
        ref_ptr(ref_ptr<R>&& rhs) noexcept :
            _ptr(rhs._ptr)
        {
            rhs._ptr = nullptr;
        }

        template<class R>
        ref_ptr(const ref_ptr<R>& ptr) noexcept :
            _ptr(ptr._ptr)
        {
            if (_ptr) _ptr->ref();
        }

        explicit ref_ptr(T* ptr) noexcept :
            _ptr(ptr)
        {
            if (_ptr) _ptr->ref();
        }

        template<class R>
        explicit ref_ptr(R* ptr) noexcept :
            _ptr(ptr)
        {
            if (_ptr) _ptr->ref();
        }

        ~ref_ptr()
        {
            if (_ptr) _ptr->unref();
        }

        ref_ptr& operator=(T* ptr)
        {
            if (ptr == _ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref in case the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }

        ref_ptr& operator=(const ref_ptr& rhs)
        {
            if (rhs._ptr == _ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = rhs._ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref in case the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }

        template<class R>
        ref_ptr& operator=(const ref_ptr<R>& rhs)
        {
            if (rhs._ptr == _ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = rhs._ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref in case the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }

        /// move assignment
        template<class R>
        ref_ptr& operator=(ref_ptr<R>&& rhs)
        {
            if (rhs._ptr == _ptr) return *this;

            if (_ptr) _ptr->unref();

            _ptr = rhs._ptr;

            rhs._ptr = nullptr;

            return *this;
        }

        template<class R>
        bool operator<(const ref_ptr<R>& rhs) const { return (rhs._ptr < _ptr); }

        template<class R>
        bool operator==(const ref_ptr<R>& rhs) const { return (rhs._ptr == _ptr); }

        template<class R>
        bool operator!=(const ref_ptr<R>& rhs) const { return (rhs._ptr != _ptr); }

        template<class R>
        bool operator<(const R* rhs) const { return (rhs < _ptr); }

        template<class R>
        bool operator==(const R* rhs) const { return (rhs == _ptr); }

        template<class R>
        bool operator!=(const R* rhs) const { return (rhs != _ptr); }

        bool valid() const noexcept { return _ptr != nullptr; }

        explicit operator bool() const noexcept { return valid(); }

        // potentially dangerous automatic type conversion, could cause dangling pointer if ref_ptr<> assigned to C pointer, if ref_ptr<> destruction cause an object delete.
        operator T*() const noexcept { return _ptr; }

        void operator[](int) const = delete;

        T& operator*() const noexcept { return *_ptr; }

        T* operator->() const noexcept { return _ptr; }

        T* get() const noexcept { return _ptr; }

        T* release_nodelete() noexcept
        {
            T* temp_ptr = _ptr;

            if (_ptr) _ptr->unref_nodelete();
            _ptr = nullptr;

            return temp_ptr;
        }

        void swap(ref_ptr& rhs) noexcept
        {
            T* temp_ptr = _ptr;
            _ptr = rhs._ptr;
            rhs._ptr = temp_ptr;
        }

        template<class R>
        ref_ptr<R> cast() const { return ref_ptr<R>(_ptr ? _ptr->template cast<R>() : nullptr); }

    protected:
        template<class R>
        friend class ref_ptr;

        T* _ptr;
    };

} // namespace vsg
