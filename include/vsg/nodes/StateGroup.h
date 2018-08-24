#pragma once

#include <vsg/nodes/Group.h>

namespace vsg
{
    // forward declare
    class State;

    class StateComponent : public Object
    {
    public:
        StateComponent() {}

        virtual void push(State& state) = 0;
        virtual void pop(State& state) = 0;

    protected:
        virtual ~StateComponent() {}
    };

    class StateGroup : public Group
    {
    public:
        StateGroup();

        virtual void accept(Visitor& visitor) override { visitor.apply(*this); }

        using StateComponents = std::vector<ref_ptr<StateComponent>>;

        void add(StateComponent* component) { _stateComponents.push_back(component); }

        inline void push(State& state) { for(auto& component : _stateComponents) component->push(state); }
        inline void pop(State& state) { for(auto& component : _stateComponents) component->pop(state); }

    protected:
        virtual ~StateGroup();

        StateComponents _stateComponents;
    };
}
