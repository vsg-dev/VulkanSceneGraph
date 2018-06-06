#pragma once

#include "Object.hpp"

namespace vsg
{
    class Node : public vsg::Object
    {
    public:
        Node();

    protected:

        virtual ~Node();
    };
}
