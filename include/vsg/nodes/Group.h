#pragma once

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <vector>

namespace vsg
{
    class Group : public vsg::Node
    {
    public:
        Group();

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            for (auto child : _children)
            {
                if (child.valid()) child->accept(visitor);
            }
        }

        std::size_t addChild(vsg::Node* child) { std::size_t pos = _children.size(); _children.push_back(child); return pos; }

        void removeChild(std::size_t pos) { _children.erase(_children.begin()+pos); }

        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return _children.size(); }

        using Children = std::vector< ref_ptr< vsg::Node> >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        virtual ~Group();

        Children _children;
    };
}
