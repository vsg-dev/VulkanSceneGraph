/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/vk/State.h>

using namespace vsg;

State::State(const Slots& in_maxSlots) :
    dirty(false)
{
    reserve(in_maxSlots);
}

void State::reserve(const Slots& in_maxSlots)
{
    maxSlots = in_maxSlots;
    activeMaxStateSlot = maxSlots.max();

    size_t required_size = static_cast<size_t>(activeMaxStateSlot) + 1;
    if (required_size > stateStacks.size()) stateStacks.resize(required_size);

    //    info("State::reserve(", maxStateSlot, ", ", maxViewSlot, ")");
}

void State::reset()
{
    //info("State::reset()");
    for (auto& stateStack : stateStacks)
    {
        stateStack.reset();
    }

    activeMaxStateSlot = maxSlots.max();
}

void State::connect(ref_ptr<CommandBuffer> commandBuffer)
{
    _commandBuffer = commandBuffer;
    commandBuffer->state = this;
    reset();
}

void State::pushView(ref_ptr<StateCommand> command)
{
    stateStacks[command->slot].push(command);
    activeMaxStateSlot = maxSlots.max();
}

void State::popView(ref_ptr<StateCommand> command)
{
    stateStacks[command->slot].pop();
    activeMaxStateSlot = maxSlots.max();
}

void State::pushView(const View& view)
{
    //info("State::pushView(View&, ", &view, ")");
    if ((viewportStateHint & DYNAMIC_VIEWPORTSTATE) && view.camera && view.camera->viewportState) pushView(view.camera->viewportState);
}

void State::popView(const View& view)
{
    //info("State::popView(View&, ", &view, ")");
    if ((viewportStateHint & DYNAMIC_VIEWPORTSTATE) && view.camera && view.camera->viewportState) popView(view.camera->viewportState);
}
