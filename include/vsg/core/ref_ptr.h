#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <atomic>
#include <memory>
#include <type_traits>

namespace vsg
{
    // smart pointer implementation aiming to get the feature richness of std::shared_ptr with the small pointer size (i.e. the same as a C pointer) of intrusive reference-counting
    template<class T>
    class ref_ptr;

    template<class T>
    class observer_ptr;

    class RefCountBase
    {
    public:
        using RefCountType = std::atomic_uint;

        RefCountBase(const RefCountBase&) = delete;
        RefCountBase& operator=(const RefCountBase&) = delete;

        virtual ~RefCountBase() noexcept = default;

        void increment() noexcept
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
// GCC bug 107694
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
            _referenceCount.fetch_add(1, std::memory_order_relaxed);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }

        bool incrementIfNonZero() noexcept
        {
            RefCountType::value_type prevCount = _referenceCount.load();
            while (prevCount != 0)
            {
                if (_referenceCount.compare_exchange_weak(prevCount, prevCount + 1))
                    return true;
            }
            return false;
        }

        void incrementObservers() noexcept
        {
            _observerReferenceCount.fetch_add(1, std::memory_order_relaxed);
        }

        void decrement() noexcept
        {
            if (--_referenceCount == 0)
            {
                destroyResource();
                decrementObservers();
            }
        }

        void decrementObservers() noexcept
        {
            if (--_observerReferenceCount == 0)
            {
                destroyRefCount();
            }
        }

        RefCountType::value_type useCount() const noexcept
        {
            return _referenceCount;
        }

    protected:
        constexpr RefCountBase() noexcept = default;

        template<class T>
        void callDestructor(T& instance)
        {
            instance.callDestructor();
        }

        template<class T>
        void callDeleteOperator(T& instance)
        {
            instance.callDeleteOperator();
        }

        template<class T>
        void assignTo(T& instance)
        {
            // assert(instance._referenceCount == nullptr || instance._referenceCount == this);
            instance._referenceCount = this;
        }

    private:
        virtual void destroyResource() noexcept = 0;
        virtual void destroyRefCount() noexcept = 0;

        RefCountType _referenceCount = 1;
        RefCountType _observerReferenceCount = 1;
    };

    // we've got lots of template magic to make this work, but don't want to pollute the main namespace
    namespace detail
    {
        template<class T>
        class RefCountPointer : public RefCountBase
        {
        public:
            explicit RefCountPointer(T* ptr) :
                RefCountBase(),
                _ptr(ptr)
            {
                assignTo(*_ptr);
            }

            template<class... Args>
            explicit RefCountPointer(Args&&... args) :
                RefCountBase(),
                _ptr(new T(this, std::forward<Args>(args)...))
            {
                assignTo(*_ptr);
            }

            T* _ptr;

        private:
            void destroyResource() noexcept override
            {
                callDeleteOperator(*_ptr);
            }

            void destroyRefCount() noexcept override
            {
                delete this;
            }
        };

        template<class T, class = void>
        struct NeedsRefCountEarly : std::false_type
        {
        };

        template<class T>
        struct NeedsRefCountEarly<T, std::void_t<typename T::NeedsRefCountInConstructor>>
            : T::NeedsRefCountInConstructor
        {
        };

        template<class Res, class Deleter>
        class RefCountResource : public RefCountBase
        {
        public:
            RefCountResource(Res resource, Deleter deleter) :
                RefCountBase(),
                _resource(resource),
                _deleter(deleter)
            {}

            ~RefCountResource() noexcept override = default;

        private:
            void destroyResource() noexcept override
            {
                _deleter(_resource);
            }

            void destroyRefCount() noexcept override
            {
                delete this;
            }

            // todo: compressed pair/no_unique_address
            Res _resource;
            Deleter _deleter;
        };

        // RefCountResourceWithAllocator

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4624) // '%s': destructor was implicitly defined as deleted
#endif
        template<class T>
        class RefCountWithObject : public RefCountBase
        {
        public:
            static void* operator new(std::size_t count)
            {
                return T::operator new(count);
            }

            static void operator delete(void* ptr)
            {
                T::operator delete(ptr);
            }

            template<class... Args>
            explicit RefCountWithObject(Args&&... args) :
                RefCountBase()
            {
                if constexpr (NeedsRefCountEarly<T>::value)
                {
                    ::new ((void*)&storage) T(this, std::forward<Args>(args)...);
                }
                else
                {
                    ::new ((void*)&storage) T(std::forward<Args>(args)...);
                }
                assignTo(storage);
            }

            ~RefCountWithObject() noexcept override
            {
                // can't use default keyword due to the union, but we don't want to do anything here
            }

            // trick to prevent double-destruction
            union
            {
                std::remove_cv_t<T> storage;
            };

        private:
            void destroyResource() noexcept override
            {
                callDestructor(storage);
            }

            void destroyRefCount() noexcept override
            {
                delete this;
            }
        };

        template<class T, class Alloc>
        class RefCountWithObjectAndAllocator : public RefCountBase
        {
        private:
            static_assert(std::is_same_v<T, std::remove_cv_t<T>>, "allocate_shared didn't remove_cv_t");

        public:
            using Rebound = typename std::allocator_traits<Alloc>::template rebind_alloc<RefCountWithObjectAndAllocator>;

            template<class... Args>
            explicit RefCountWithObjectAndAllocator(const Alloc& alloc, Args&&... args) :
                RefCountBase(),
                allocator(alloc)
            {
                // roughly equivalent to
                // alloc.construct(&storage, std::forward<Args>(args)...);
                // we can't use that, though, as vsg::Object destructor is protected, so std::is_constructible_v is false
                if constexpr (!std::uses_allocator_v<T, decltype(alloc)>)
                {
                    ::new ((void*)&storage) T(std::forward<Args>(args)...);
                }
                else
                {
                    ::new ((void*)&storage) T(std::forward<Args>(args)..., alloc);
                }
                assignTo(storage);
            }

            ~RefCountWithObjectAndAllocator() noexcept
            {
                // can't use default keyword due to the union, but we don't want to do anything here
            }

            // trick to prevent double-destruction
            union
            {
                T storage;
            };

            // todo: empty base optimisation/no_unique_address
            Rebound allocator;

        private:
            void destroyResource() noexcept override
            {
                callDestructor(storage);
            }

            void destroyRefCount() noexcept override
            {
                this->~RefCountWithObjectAndAllocator();
                std::allocator_traits<Rebound>::deallocate(allocator, this, 1);
            }
        };
#ifdef _MSC_VER
#    pragma warning(pop)
#endif

        template<class T>
        struct TemporaryOwner
        {
            T* ptr = nullptr;
        };
    } // namespace detail

    template <class T>
    class ref_ptr
    {
    public:
        using element_type = T;
        using observer_type = observer_ptr<T>;

        constexpr ref_ptr() noexcept = default;

        constexpr ref_ptr(std::nullptr_t) noexcept {}

        ref_ptr(detail::TemporaryOwner<T>& ptr) noexcept :
            _ptr(ptr.ptr)
        {
            ptr.ptr = nullptr;
        }

        explicit ref_ptr(T* ptr) noexcept :
            _ptr(ptr)
        {
            if (_ptr)
            {
                if (_ptr->_referenceCount)
                    _ptr->_referenceCount->increment();
                else
                    _ptr->_referenceCount = new detail::RefCountPointer(_ptr);
            }
        }

        ref_ptr& operator=(T* ptr)
        {
            if (ptr == _ptr) return *this;

            T* temp_ptr = _ptr;

            _ptr = ptr;

            if (_ptr)
            {
                if (_ptr->_referenceCount)
                    _ptr->_referenceCount->increment();
                else
                    _ptr->_referenceCount = new detail::RefCountPointer(_ptr);
            }

            // unref the original pointer after ref in case the old pointer object is a parent of the new pointer's object
            if (temp_ptr) temp_ptr->_referenceCount->decrement();

            return *this;
        }

        ref_ptr(const ref_ptr& rhs) noexcept:
            _ptr(rhs._ptr)
        {
            if (_ptr)
                _ptr->_referenceCount->increment();
        }

        ref_ptr& operator=(const ref_ptr& rhs) noexcept
        {
            ref_ptr(rhs).swap(*this);
            return *this;
        }

        template<class R>
        ref_ptr(const ref_ptr<R>& rhs) noexcept :
            _ptr(rhs._ptr)
        {
            if (_ptr)
                _ptr->_referenceCount->increment();
        }

        template<class R>
        ref_ptr& operator=(const ref_ptr<R>& rhs) noexcept
        {
            ref_ptr(rhs).swap(*this);
            return *this;
        }

        ~ref_ptr() noexcept
        {
            if (_ptr)
                _ptr->_referenceCount->decrement();
        }

        void reset() noexcept
        {
            ref_ptr().swap(*this);
        }

        void swap(ref_ptr& other) noexcept
        {
            std::swap(_ptr, other._ptr);
        }

        [[nodiscard]] element_type* get() const noexcept
        {
            return _ptr;
        }

        bool valid() const noexcept
        {
            return get() != nullptr;
        }

        explicit operator bool() const noexcept
        {
            return valid();
        }

        T& operator*() const noexcept { return *_ptr; }

        T* operator->() const noexcept { return _ptr; }

        // potentially dangerous automatic type conversion, could cause dangling pointer if ref_ptr<> assigned to C pointer and ref_ptr<> destruction causes an object delete.
        operator T*() const noexcept { return _ptr; }

        void operator[](int) const = delete;

        template<class R>
        ref_ptr<R> cast() const { return ref_ptr<R>(_ptr ? _ptr->template cast<R>() : nullptr); }

    private:
        template<class R>
        friend class ref_ptr;

        element_type* _ptr = nullptr;
    };

    // like std::make_shared, but allocates the ref count separately
    template<class T, class... Args>
    [[nodiscard]] ref_ptr<T> make_referenced(Args&&... args)
    {
        if constexpr (detail::NeedsRefCountEarly<T>::value)
        {
            auto* controlBlock = new detail::RefCountPointer<T>(std::forward<Args>(args)...);
            detail::TemporaryOwner<T> temp{controlBlock->_ptr};
            return ref_ptr<T>(temp);
        }
        else
        {
            auto* instance = new T(std::forward<Args>(args)...);
            new detail::RefCountPointer<T>(instance);
            detail::TemporaryOwner<T> temp{instance};
            return ref_ptr<T>(temp);
        }
    }

    // like std::make_shared
    template<class T, class... Args>
    [[nodiscard]] ref_ptr<T> make_referenced_adjacent_ref_count(Args&&... args)
    {
        auto* controlBlock = new detail::RefCountWithObject<T>(std::forward<Args>(args)...);
        detail::TemporaryOwner<T> temp{std::addressof(controlBlock->storage)};
        return ref_ptr<T>(temp);
    }

    // like std::allocate_shared
    template<class T, class Alloc, class... Args>
    [[nodiscard]] ref_ptr<T> allocate_referenced_adjacent_ref_count(const Alloc& allocator, Args&&... args)
    {
        using ControlBlock = detail::RefCountWithObjectAndAllocator<std::remove_cv_t<T>, Alloc>;
        auto reboundAlloc = typename ControlBlock::Rebound(allocator);
        ControlBlock* controlBlock = std::allocator_traits<typename ControlBlock::Rebound>::allocate(reboundAlloc, 1);
        ::new ((void*)controlBlock) ControlBlock(reboundAlloc, std::forward<Args>(args)...);
        detail::TemporaryOwner<T> temp{std::addressof(controlBlock->storage)};
        return ref_ptr<T>(temp);
    }

} // namespace vsg
