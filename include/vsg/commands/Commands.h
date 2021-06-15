#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <vsg/commands/Command.h>
#include <vsg/nodes/Node.h>

#include <vector>

namespace vsg
{

    class VSG_DECLSPEC Commands : public Inherit<Command, Commands>
    {
    public:
        Commands(size_t numChildren = 0);
        Commands(Allocator* allocator, size_t numChildren = 0);

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node.children) child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        using Children = std::vector<ref_ptr<vsg::Command>>;
        Children children;

        void addChild(vsg::ref_ptr<Command> child)
        {
            children.push_back(child);
        }

#if VSG_USE_DEPRECATED_METHODS_AND_IO
        std::size_t addChild(vsg::ref_ptr<Command> child)
        {
            std::size_t pos = children.size();
            children.push_back(child);
            return pos;
        }

        void removeChild(std::size_t pos) { children.erase(children.begin() + pos); }

        void setChild(std::size_t pos, Command* node) { children[pos] = node; }
        vsg::Command* getChild(std::size_t pos) { return children[pos].get(); }
        const vsg::Command* getChild(std::size_t pos) const { return children[pos].get(); }

        std::size_t getNumChildren() const noexcept { return children.size(); }

        using Children = std::vector<ref_ptr<vsg::Command>>;

        Children& getChildren() noexcept { return children; }
        const Children& getChildren() const noexcept { return children; }
#endif

        void compile(Context& context) override;
        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~Commands();
    };
    VSG_type_name(vsg::Commands);

} // namespace vsg
