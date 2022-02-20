#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

namespace vsg
{
    class VSG_DECLSPEC Node : public Inherit<Object, Node>
    {
    public:
        Node(Allocator* allocator = nullptr);

        static void* operator new(std::size_t count);
        static void operator delete(void* ptr);

    protected:
        virtual ~Node();
    };
    VSG_type_name(vsg::Node);

    template<typename T>
    struct node_allocator
    {
        using value_type = T;

        node_allocator() = default;
        template <class U> constexpr node_allocator(const node_allocator<U>&) noexcept {}

        value_type* allocate(std::size_t n)
        {
            const std::size_t size = n * sizeof(value_type);
            return static_cast<value_type*>(Node::operator new(size));
        }

        void deallocate(value_type* ptr, std::size_t /*n*/)
        {
            Node::operator delete(ptr);
        }
    };

    template <class T, class U>
    bool operator==(const node_allocator <T>&, const node_allocator <U>&) { return true; }

    template <class T, class U>
    bool operator!=(const node_allocator <T>&, const node_allocator <U>&) { return false; }

} // namespace vsg
