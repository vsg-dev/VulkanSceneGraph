#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Visitor.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/traversals/DispatchTraversal.h>
#include <vsg/traversals/CullTraversal.h>

namespace vsg
{

    // Use the Curiously Recurring Template Pattern
    // to provide the classes versions of accept(..) and sizeofObject()
    template<class ParentClass, class Subclass>
    class Inherit : public ParentClass
    {
    public:
        template<typename... Args>
        Inherit(Args&&... args) : ParentClass(std::forward<Args>(args)...) {}

        std::size_t sizeofObject() const noexcept override { return sizeof(Subclass); }

        void accept(Visitor& visitor) override { visitor.apply(static_cast<Subclass&>(*this)); }
        void accept(ConstVisitor& visitor) const override { visitor.apply(static_cast<const Subclass&>(*this)); }
        void accept(DispatchTraversal& visitor) const override { visitor.apply(static_cast<const Subclass&>(*this)); }
        void accept(CullTraversal& visitor) const override { visitor.apply(static_cast<const Subclass&>(*this)); }
    };

}
