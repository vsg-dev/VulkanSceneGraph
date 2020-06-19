#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

#include <vsg/io/stream.h>

#include <vsg/traversals/RecordTraversal.h>

namespace vsg
{

    class VSG_DECLSPEC Allocator : public Object
    {
    public:
        std::size_t sizeofObject() const noexcept override { return sizeof(Allocator); }

        void accept(Visitor& visitor) override { visitor.apply(static_cast<Allocator&>(*this)); }
        void accept(ConstVisitor& visitor) const override { visitor.apply(static_cast<const Allocator&>(*this)); }
        void accept(RecordTraversal& visitor) const override { visitor.apply(static_cast<const Allocator&>(*this)); }

        virtual void* allocate(std::size_t n, const void* hint);

        virtual void* allocate(std::size_t size);

        virtual void deallocate(const void* ptr, std::size_t size = 0);

        template<typename T, typename... Args>
        T* newObject(Args... args)
        {
            void* ptr = allocate(sizeof(T));
            if (ptr)
            {
                T* t = new (ptr) T(args...);
                return t;
            }
            return nullptr;
        }

        template<typename T>
        void deleteObject(T* ptr)
        {
            if (ptr)
            {
                ptr->~T();
                deallocate(ptr, sizeof(T));
            }
        }

        template<typename T>
        T* newArray(size_t size)
        {
            void* ptr = allocate(size * sizeof(T));
            if (ptr)
            {
                T* t = new (ptr) T[size];
                return t;
            }
            return nullptr;
        }

        template<typename T>
        void deleteArray(T* ptr, size_t size)
        {
            if (ptr)
            {
                for (size_t i = 0; i < size; ++i)
                {
                    (ptr[i]).~T();
                }
                deallocate(ptr, size * sizeof(T));
            }
        }

        Auxiliary* getOrCreateSharedAuxiliary();

        void detachSharedAuxiliary(Auxiliary* auxiliary);

    protected:
        virtual ~Allocator();

        Auxiliary* _sharedAuxiliary = nullptr;
        std::size_t _bytesAllocated = 0;
        std::size_t _countAllocated = 0;
        std::size_t _bytesDeallocated = 0;
        std::size_t _countDeallocated = 0;
    };

} // namespace vsg
