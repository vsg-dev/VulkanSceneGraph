/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/State.h>
#include <vsg/app/View.h>

using namespace vsg;

State::State(uint32_t maxSlot) :
    dirty(false)
{
    reserve(maxSlot);
}

void State::reserve(uint32_t maxSlot)
{
    size_t required_size = static_cast<size_t>(maxSlot) + 1;
    if (required_size > stateStacks.size()) stateStacks.resize(required_size);

    if (required_size > viewStateStacks.size()) viewStateStacks.resize(required_size);

    //info("State::reserve(", maxSlot, ")");
}


void State::pushView(ref_ptr<StateCommand> command)
{
    viewStateStacks[command->slot].push(command);
    recordView();
}

void State::popView(ref_ptr<StateCommand> command)
{
    viewStateStacks[command->slot].pop();
    recordView();
}

void State::pushView(const View& view)
{
    //info("State::pushView(View&, ", &view, ")");
    if (view.camera && view.camera->viewportState) pushView(view.camera->viewportState);
}

void State::popView(const View& view)
{
    //info("State::popView(View&, ", &view, ")");
    if (view.camera && view.camera->viewportState) pushView(view.camera->viewportState);
}

void State::recordView()
{
    //info("State::recordView(..)");

    for (auto& stateStack : viewStateStacks)
    {
        if (!stateStack.empty()) stateStack.record(*_commandBuffer);
    }
}
