#pragma once

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <vector>
#include <algorithm>

#define INLINE_TRAVERSE

namespace vsg
{
    class Group : public vsg::Node
    {
    public:
        Group();

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

#ifdef INLINE_TRAVERSE

        inline virtual void traverse(Visitor& visitor)
        {
            std::for_each(_children.begin(), _children.end(), [&visitor](ref_ptr<Node>& child)
            {
                child->accept(visitor);
            });
        }
#else
        virtual void traverse(Visitor& visitor);
#endif

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
