#pragma once

#include <vsg/core/Value.h>

namespace vsg
{

    // forward declare nodes
    class Node;
    class Group;
    class QuadGroup;
    class LOD;

    class Visitor
    {
    public:

        Visitor();

        virtual void apply(Object&);

        // Values
        virtual void apply(StringValue&);
        virtual void apply(IntValue&);
        virtual void apply(UIntValue&);
        virtual void apply(FloatValue&);
        virtual void apply(DoubleValue&);

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Group&);
        virtual void apply(QuadGroup&);
        virtual void apply(LOD&);
    };


    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(Visitor& visitor) { visitor.apply(*this); }

}
