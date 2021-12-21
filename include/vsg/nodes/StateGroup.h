#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/state/StateCommand.h>
#include <vsg/traversals/ArrayState.h>
#include <vsg/traversals/CompileTraversal.h>

#include <algorithm>

namespace vsg
{
    // forward declare
    class CommandBuffer;

    class VSG_DECLSPEC StateGroup : public Inherit<Group, StateGroup>
    {
    public:
        StateGroup(Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateCommands = std::vector<ref_ptr<StateCommand>>;
        StateCommands stateCommands;

        /// if the shaders associated with GraphicsPipeline don't treat the array 0 as xyz vertex then provide a ArrayState prototype to provide custom mapping of arrays to vertices.
        ref_ptr<ArrayState> prototypeArrayState;

        template<class T>
        bool contains(const T value) const
        {
            return std::find(stateCommands.begin(), stateCommands.end(), value) != stateCommands.end();
        }

        void add(ref_ptr<StateCommand> stateCommand)
        {
            stateCommands.push_back(stateCommand);
        }

        template<class T>
        void remove(const T value)
        {
            if (auto itr = std::find(stateCommands.begin(), stateCommands.end(), value); itr != stateCommands.end())
            {
                stateCommands.erase(itr);
            }
        }

        virtual void compile(Context& context);

    protected:
        virtual ~StateGroup();
    };
    VSG_type_name(vsg::StateGroup);

} // namespace vsg
