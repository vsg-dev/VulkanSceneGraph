#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/StateCommand.h>

namespace vsg
{
    class VSG_DECLSPEC StateSwitch : public Inherit<StateCommand, StateSwitch>
    {
    public:
        template<class N, class V>
        static void t_traverse(N& sc, V& visitor)
        {
            for (auto& child : sc.children)
            {
                if ((visitor.traversalMask & (visitor.overrideMask | child.mask)) != MASK_OFF) child.stateCommand->accept(visitor);
            }
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void compile(Context& context) override;
        void record(CommandBuffer& commandBuffer) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void add(Mask mask, ref_ptr<StateCommand> sc) { children.emplace_back(Child{mask, sc}); }

        struct Child
        {
            Mask mask = MASK_ALL;
            ref_ptr<StateCommand> stateCommand;
        };

        std::vector<Child> children;
    };
    VSG_type_name(vsg::StateSwitch);

} // namespace vsg
