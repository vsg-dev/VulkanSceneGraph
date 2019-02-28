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

    // scene graph interface for encapsulating the creation Descriptor's such as for wrapping uniforms and textures
    class StateAttribute : public Inherit<Object, StateAttribute>
    {
    public:
        StateAttribute(Allocator* allocator = nullptr) : Inherit(allocator) {}

        virtual ref_ptr<vsg::Descriptor> compile(Context& /*context*/) = 0;

    protected:
        virtual ~StateAttribute() {}
    };
    VSG_type_name(vsg::StateAttribute);

    // scene graph interface for enacpsulating the binding of StateAttributes togther into a single VkCmdBindDescriptorSets/VkDescriptorSets
    class VSG_DECLSPEC StateSet : public Inherit<StateCommand, StateSet>
    {
    public:
        StateSet(Allocator* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        using StateAttributes = std::vector<ref_ptr<StateAttribute>>;

        virtual void compile(Context& context);

        virtual void pushTo(State& state) const
        {
            _bindDescriptorSets->pushTo(state);
        }

        virtual void popFrom(State& state) const
        {
            _bindDescriptorSets->pushTo(state);
        }

        inline void add(ref_ptr<StateAttribute> attribute)
        {
            _attributes.push_back(attribute);
        }

        void dispatch(CommandBuffer& commandBuffer) const override
        {
            _bindDescriptorSets->dispatch(commandBuffer);
        }

        StateAttributes _attributes;
        VkPipelineBindPoint _bindPoint;
        uint32_t _firstSet;

    protected:
        virtual ~StateSet();


        ref_ptr<BindDescriptorSets> _bindDescriptorSets;
    };
    VSG_type_name(vsg::StateSet);


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
