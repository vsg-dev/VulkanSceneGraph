#pragma once

namespace vsg
{

    template<class T>
    class ref_ptr
    {
    public:

        ref_ptr() : _ptr(nullptr) {}

        ref_ptr(const ref_ptr& rhs) :
            _ptr(rhs._ptr)
        {
            if (_ptr) _ptr->ref();
        }

        template<class R>
        ref_ptr(R* ptr):
            _ptr(ptr)
        {
            if (_ptr) _ptr->ref();
        }

        ~ref_ptr()
        {
            if (_ptr) _ptr->unref();
        }

        ref_ptr& operator = (T* ptr)
        {
            if (ptr==_ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref incase the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }

        ref_ptr& operator = (const ref_ptr& rhs)
        {
            if (rhs._ptr==_ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = rhs._ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref incase the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }


        template<class R>
        ref_ptr& operator = (const ref_ptr<R>& rhs)
        {
            if (rhs._ptr==_ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = rhs._ptr;

            if (_ptr) _ptr->ref();

            // unref the original pointer after ref incase the old pointer object a parent of the new pointers object
            if (temp_ptr) temp_ptr->unref();

            return *this;
        }

        T& operator*() const
        {
            return *_ptr;
        }

        T* operator->() const
        {
            return _ptr;
        }


        T* get() const { return _ptr; }

        T* release()
        {
            T* temp_ptr = _ptr;

            if (_ptr) _ptr->unref_nodelete();
            _ptr = nullptr;

            return temp_ptr;
        }

        void swap(ref_ptr& rhs)
        {
            T* temp_ptr = _ptr;
            _ptr = rhs._ptr;
            rhs._ptr = temp_ptr;
        }

    protected:

        template<class R> friend class ref_ptr;

        T* _ptr;

    };

}
