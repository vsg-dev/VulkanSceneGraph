#pragma once

#include <vsg/nodes/Group.h>

namespace vsg
{
    // forward declare
    class State;
    class CommandBuffer;

    class StateComponent : public Object
    {
    public:
        StateComponent() {}

        virtual void pushTo(State& state) = 0;
        virtual void popFrom(State& state) = 0;

        virtual void dispatch(CommandBuffer& commandBuffer) const = 0;

    protected:
        virtual ~StateComponent() {}
    };

    class VSG_EXPORT StateGroup : public Group
    {
    public:
        StateGroup();

        virtual void accept(Visitor& visitor) override { visitor.apply(*this); }

        using StateComponents = std::vector<ref_ptr<StateComponent>>;

        void add(StateComponent* component) { _stateComponents.push_back(component); }

        inline void pushTo(State& state) { for(auto& component : _stateComponents) component->pushTo(state); }
        inline void popFrom(State& state) { for(auto& component : _stateComponents) component->popFrom(state); }

    protected:
        virtual ~StateGroup();

        StateComponents _stateComponents;
    };
}
