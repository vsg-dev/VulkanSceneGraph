#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

    /////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // vsg::visit<> variants construct a Visitor and pass it to one or more objects accept() method
    //

    /// helper function that default constructors visitor, calls accept() on each of the objects in specified range and returns the visitor so it can be queried for any results or reused.
    /// usage: auto matrix = vsg::visit<vsg::ComputeTransform>(objects.begin(), objects.end()).matrix;
    template<class V, typename I>
    V visit(I begin, I end)
    {
        V visitor;
        for (I itr = begin; itr != end; ++itr)
        {
            (*itr)->accept(visitor);
        }
        return visitor;
    }

    /// helper function that default constructors visitor, calls accept() on specified object and returns the visitor so it can be queried for any results or reused.
    /// usage: auto matrix = vsg::visit<vsg::ComputeTransform>(object).matrix;
    template<class V, typename P>
    V visit(vsg::ref_ptr<P> ptr)
    {
        V visitor;
        if (ptr) ptr->accept(visitor);
        return visitor;
    }

    /// helper function that default constructors visitor, calls accept() on specified object and returns the visitor so it can be queried for any results or reused.
    /// usage: auto matrix = vsg::visit<vsg::ComputeTransform>(object).matrix;
    template<class V, typename P>
    V visit(P* ptr)
    {
        V visitor;
        if (ptr) ptr->accept(visitor);
        return visitor;
    }

    /// helper function that default constructors visitor, calls accept() on all the objects in a const container and returns the visitor so it can be queried for any results or reused.
    /// usage: auto matrix = vsg::visit<vsg::ComputeTransform>(nodePath).matrix;
    template<class V, typename C>
    V visit(const C& container)
    {
        V visitor;
        for (const auto& object : container)
        {
            object->accept(visitor);
        }
        return visitor;
    }

    /// helper function that default constructors visitor, calls accept() on all the objects in a container and returns the visitor so it can be queried for any results or reused.
    /// usage: auto matrix = vsg::visit<vsg::ComputeTransform>(nodePath).matrix;
    template<class V, typename C>
    V visit(C& container)
    {
        V visitor;
        for (auto& object : container)
        {
            object->accept(visitor);
        }
        return visitor;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // vsg::visit<> variants that pass an existing Visitor to one or more objects accept() method, returning a reference to the visitor
    //

    /// helper function calls accept(visitor) on each of the objects in specified range and returns a reference to the visitor so it can be queried for any results or reused.
    /// usage:
    ///      vsg::ComputeTransformusage ct;
    ///      auto matrix = vsg::visit(ct, objects.begin(), objects.end()).matrix;
    template<class V, typename I>
    V& visit(V& visitor, I begin, I end)
    {
        for (I itr = begin; itr != end; ++itr)
        {
            (*itr)->accept(visitor);
        }
        return visitor;
    }

    /// helper function calls accept(visitor) on specified object and returns a reference to the visitor so it can be queried for any results or reused.
    /// usage:
    ///      vsg::ComputeTransformusage ct;
    ///      auto matrix = vsg::visit(ct, object).matrix;
    template<class V, typename P>
    V& visit(V& visitor, vsg::ref_ptr<P> ptr)
    {
        if (ptr) ptr->accept(visitor);
        return visitor;
    }

    /// helper function calls accept(visitor) on specified object and returns a reference to the visitor so it can be queried for any results or reused.
    /// usage:
    ///      vsg::ComputeTransformusage ct;
    ///      auto matrix = vsg::visit(ct, object).matrix;
    template<class V, typename P>
    V& visit(V& visitor, P* ptr)
    {
        if (ptr) ptr->accept(visitor);
        return visitor;
    }

    /// helper function calls accept(visitor) on all the objects in a const container and returns a reference to the visitor so it can be queried for any results or reused.
    /// usage:
    ///      vsg::ComputeTransformusage ct;
    ///      auto matrix = vsg::visit(ct, container).matrix;
    template<class V, typename C>
    V& visit(V& visitor, const C& container)
    {
        for (const auto& object : container)
        {
            object->accept(visitor);
        }
        return visitor;
    }

    /// helper function calls accept(visitor) on all the objects in a container and returns a reference to the visitor so it can be queried for any results or reused.
    /// usage:
    ///      vsg::ComputeTransformusage ct;
    ///      auto matrix = vsg::visit(ct, container).matrix;
    template<class V, typename C>
    V& visit(V& visitor, C& container)
    {
        for (auto& object : container)
        {
            object->accept(visitor);
        }
        return visitor;
    }

} // namespace vsg
