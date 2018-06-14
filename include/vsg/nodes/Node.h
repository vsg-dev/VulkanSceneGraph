#pragma once

#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>

namespace vsg
{
    class Node : public vsg::Object
    {
    public:
        Node();

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

    protected:

        virtual ~Node();
    };
}
