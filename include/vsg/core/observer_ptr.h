#pragma once

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
            _auxiliary(ptr ? ptr->getOrCreateAuxiliary() : nullptr)
        {
        }

        template<class R>
        observer_ptr(const observer_ptr<R>& ptr):
            _auxiliary(ptr._auxiliary)
        {
        }

        template<class R>
        observer_ptr(const ref_ptr<R>& ptr):
            _auxiliary(ptr.valid() ? ptr->getOrCreateAuxiliary() : nullptr)
        {
        }

        ~observer_ptr()
        {
        }

        template<class R>
        observer_ptr& operator = (R* ptr)
        {
            _auxiliary = ptr ? ptr->getOrCreateAuxiliary() : nullptr;
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
            _auxiliary = rhs.valid() ? rhs->getOrCreateAuxiliary() : nullptr;
            return *this;
        }

        bool valid() const { return _auxiliary.valid() && _auxiliary->getConnectedObject()!=nullptr; }

        bool operator!() const { return !_auxiliary || _auxiliary->getConnectedObject()==nullptr; }

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
