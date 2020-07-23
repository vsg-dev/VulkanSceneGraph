#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/type_name.h>

#include <vsg/traversals/RecordTraversal.h>

namespace vsg
{

    // Use the Curiously Recurring Template Pattern
    // to provide the classes versions of accept(..) and sizeofObject()
    template<class ParentClass, class Subclass>
    class Inherit : public ParentClass
    {
    public:
        template<typename... Args>
        Inherit(Allocator* allocator, Args&&... args) :
            ParentClass(allocator, args...) {}

        template<typename... Args>
        Inherit(Args&&... args) :
            ParentClass(args...) {}

        template<typename... Args>
        static ref_ptr<Subclass> create(ref_ptr<Allocator> allocator, Args&&... args)
        {
            if (allocator)
            {
                // need to think about alignment...
                const std::size_t size = sizeof(Subclass);
                void* ptr = allocator->allocate(size);

                ref_ptr<Subclass> object(new (ptr) Subclass(allocator, args...));
                // object->setAuxiliary(allocator->getOrCreateSharedAuxiliary());

                // check the sizeof(Subclass) is consistent with Subclass::sizeOfObject()
                if (std::size_t new_size = object->sizeofObject(); new_size != size)
                {
                    throw make_string("Warning: Allocator::create(", typeid(Subclass).name(), ") mismatch sizeof() = ", size, ", ", new_size);
                }
                return object;
            }
            else
                return ref_ptr<Subclass>(new Subclass(args...));
        }

        template<typename... Args>
        static ref_ptr<Subclass> create(Args&&... args)
        {
            return ref_ptr<Subclass>(new Subclass(args...));
        }

        template<typename... Args>
        static ref_ptr<Subclass> create_if(bool flag, Args&&... args)
        {
            if (flag) return ref_ptr<Subclass>(new Subclass(args...));
            return {};
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(Subclass); }
        const char* className() const noexcept override { return type_name<Subclass>(); }
        const std::type_info& type_info() const noexcept override { return typeid(Subclass); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Subclass) == type ? true : ParentClass::is_compatible(type); }

        void accept(Visitor& visitor) override { visitor.apply(static_cast<Subclass&>(*this)); }
        void accept(ConstVisitor& visitor) const override { visitor.apply(static_cast<const Subclass&>(*this)); }
        void accept(RecordTraversal& visitor) const override { visitor.apply(static_cast<const Subclass&>(*this)); }
    };

} // namespace vsg
