#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/utils/stream.h>

namespace vsg
{

    class Allocator : public Inherit<Object, Allocator>
    {
    public:
        virtual void* allocate(std::size_t n, const void* hint );

        virtual void* allocate(std::size_t size);

        virtual void deallocate(const void* ptr, std::size_t size=0);

        Auxiliary* getOrCreateSharedAuxiliary();

        void detachSharedAuxiliary(Auxiliary* auxiliary);

        template<typename T, typename... Args>
        ref_ptr<T> create(Args&&... args)
        {
            // need to think about alignment...
            std::size_t size = sizeof(T);
            void* ptr = allocate(size);
            T* object = new (ptr) T(std::forward<Args>(args)...);
            object->setAuxiliary(getOrCreateSharedAuxiliary());

            std::size_t new_size = object->sizeofObject();
            if (new_size != size)
            {
                throw make_string("Warning: Allocator::create(",typeid(T).name(),") mismatch sizeof() = ",size,", ",new_size);
            }
            return object;
        }

    protected:
        virtual ~Allocator();

        Auxiliary*  _sharedAuxiliary = nullptr;
        std::size_t _bytesAllocated = 0;
        std::size_t _countAllocated = 0;
        std::size_t _bytesDeallocated = 0;
        std::size_t _countDeallocated = 0;
    };

}
