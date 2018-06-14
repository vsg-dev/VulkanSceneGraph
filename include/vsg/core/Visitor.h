#pragma once

#include <vsg/core/Value.h>

namespace vsg
{

    // forward declare nodes
    class Node;
    class Group;

    class Visitor
    {
    public:

        Visitor();

        virtual void apply(Object&);

        // Values
        virtual void apply(StringValue&);
        virtual void apply(IntValue&);
        virtual void apply(FloatValue&);
        virtual void apply(DoubleValue&);

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Group&);
    };


    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(Visitor& visitor) { visitor.apply(*this); }

}
