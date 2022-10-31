#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Inherit.h>

namespace vsg
{

    /// Node base class used for internal nodes within the scene graph.
    /// Specializes new/delete so that nodes are automatically allocated alongside of other nodes via vsg::Allocator's MEMORY_NODES_OBJECTS affinity.
    class VSG_DECLSPEC Node : public Inherit<Object, Node>
    {
    public:
        Node();

        /// provide new and delete to enable custom memory management via the vsg::Allocator singleton, using the MEMORY_NODES_OBJECTS
        static void* operator new(std::size_t count);
        static void operator delete(void* ptr);

    protected:
        virtual ~Node();
    };
    VSG_type_name(vsg::Node);

} // namespace vsg
