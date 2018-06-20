#pragma once

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <array>

namespace vsg
{
    class QuadGroup : public vsg::Node
    {
    public:
        QuadGroup() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            for (auto child : _children)
            {
                if (child.valid()) child->accept(visitor);
            }
        }

        void setChild(std::size_t pos, vsg::Node* node)
        {
            _children[pos] = node;
        }

        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return 4; }

        using Children = std::array< ref_ptr< vsg::Node>, 4 >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        virtual ~QuadGroup() {}

        Children _children;
    };

}
