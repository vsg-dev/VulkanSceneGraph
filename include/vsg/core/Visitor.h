#pragma once

namespace vsg
{
    class Object;

    class StringValue;
    class IntValue;
    class FloatValue;
    class DoubleValue;

    class Node;
    class Group;

    class Visitor
    {
    public:

        Visitor();

        virtual void apply(Object&);

        // ValueObjects
        virtual void apply(StringValue&);
        virtual void apply(IntValue&);
        virtual void apply(FloatValue&);
        virtual void apply(DoubleValue&);

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Group&);
    };


}
