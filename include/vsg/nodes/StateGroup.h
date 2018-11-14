#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

namespace vsg
{
    // forward declare
    class State;
    class CommandBuffer;

    class StateComponent : public Inherit<Object, StateComponent>
    {
    public:
        StateComponent() {}

        virtual void pushTo(State& state) const = 0;
        virtual void popFrom(State& state) const = 0;

        virtual void dispatch(CommandBuffer& commandBuffer) const = 0;

    protected:
        virtual ~StateComponent() {}
    };

    class VSG_DECLSPEC StateGroup : public Inherit<Group, StateGroup>
    {
    public:
        StateGroup();

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateComponents = std::vector<ref_ptr<StateComponent>>;

#if 1
        void add(ref_ptr<StateComponent> component)
        {
            _stateComponents.push_back(component);
        }
#else
        void add(StateComponent* component)
        {
            _stateComponents.push_back(component);
        }
#endif

        inline void pushTo(State& state) const
        {
            for (auto& component : _stateComponents) component->pushTo(state);
        }
        inline void popFrom(State& state) const
        {
            for (auto& component : _stateComponents) component->popFrom(state);
        }

    protected:
        virtual ~StateGroup();

        StateComponents _stateComponents;
    };
} // namespace vsg
