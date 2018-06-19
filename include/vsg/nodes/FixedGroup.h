#pragma once

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <array>

namespace vsg
{
    template<int N>
    class FixedGroup : public vsg::Node
    {
    public:
        FixedGroup() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            for(auto child : _children)
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

        std::size_t getNumChildren() const { return N; }

        using Children = std::array< ref_ptr< vsg::Node>, N >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        virtual ~FixedGroup() {}

        Children _children;
    };

}
