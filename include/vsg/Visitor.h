#pragma once

namespace vsg
{
    class Object;
    class Node;
    class Group;

    class Visitor
    {
    public:

        Visitor();

        virtual void apply(Object& object);
        virtual void apply(Node& object);
        virtual void apply(Group& object);
    };


}
