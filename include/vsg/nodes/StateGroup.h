#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorSet.h>

#include <vsg/traversals/CompileTraversal.h>

namespace vsg
{
    // forward declare
    class State;
    class CommandBuffer;


    class VSG_DECLSPEC StateGroup : public Inherit<Group, StateGroup>
    {
    public:
        StateGroup(Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateCommands = std::vector<ref_ptr<StateCommand>>;

        void add(ref_ptr<StateCommand> stateCommand)
        {
            _stateCommands.push_back(stateCommand);
        }

        inline void pushTo(State& state) const
        {
            for(auto& stateCommand : _stateCommands)
            {
                stateCommand->pushTo(state);
            }
        }
        inline void popFrom(State& state) const
        {
            for(auto& stateCommand : _stateCommands)
            {
                stateCommand->popFrom(state);
            }
        }

        virtual void compile(Context& context);

    protected:
        virtual ~StateGroup();

        StateCommands _stateCommands;
    };
    VSG_type_name(vsg::StateGroup);

} // namespace vsg
