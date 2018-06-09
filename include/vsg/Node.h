#pragma once

#include <vsg/Object.h>

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
