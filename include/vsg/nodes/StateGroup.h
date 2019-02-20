#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/vk/Command.h>

namespace vsg
{
    // forward declare
    class State;
    class CommandBuffer;
    class Context;

    class StateComponent : public Inherit<Command, StateComponent>
    {
    public:
        StateComponent() {}

        virtual void compile(Context& /*context*/) {}

        virtual void pushTo(State& state) const = 0;
        virtual void popFrom(State& state) const = 0;

    protected:
        virtual ~StateComponent() {}
    };
    VSG_type_name(vsg::StateComponent);

    class VSG_DECLSPEC StateSet : public Inherit<Object, StateSet>
    {
    public:
        StateSet(Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateComponents = std::vector<ref_ptr<StateComponent>>;

        inline void compile(Context& context)
        {
            for(auto& component : _stateComponents)
            {
                component->compile(context);
            }
        }

        inline void pushTo(State& state) const
        {
            for(auto& component : _stateComponents)
            {
                component->pushTo(state);
            }
        }

        inline void popFrom(State& state) const
        {
            for(auto& component : _stateComponents)
            {
                component->pushTo(state);
            }
        }

        inline void add(ref_ptr<StateComponent> component)
        {
            _stateComponents.push_back(component);
        }

        StateComponents _stateComponents;

    protected:
        virtual ~StateSet();
    };


    class VSG_DECLSPEC StateGroup : public Inherit<Group, StateGroup>
    {
    public:
        StateGroup(Allocator* allocator = nullptr);

        StateGroup(StateSet* stateset);

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateComponents = std::vector<ref_ptr<StateComponent>>;

        void add(ref_ptr<StateComponent> component)
        {
            _stateset->add(component);
        }

        StateSet* getStateSet() { return _stateset; }
        const StateSet* getStateSet() const { return _stateset; }

        inline void pushTo(State& state) const
        {
            _stateset->pushTo(state);
        }
        inline void popFrom(State& state) const
        {
            _stateset->popFrom(state);
        }

    protected:
        virtual ~StateGroup();

        ref_ptr<StateSet> _stateset;
    };
    VSG_type_name(vsg::StateGroup);

} // namespace vsg
