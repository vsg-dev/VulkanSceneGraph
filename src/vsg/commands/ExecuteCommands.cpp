/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/ExecuteCommands.h>

using namespace vsg;

ExecuteCommands::ExecuteCommands() :
    _latch(vsg::Latch::create(0))
{
}

ExecuteCommands::~ExecuteCommands()
{
    // disconnect all the CommandGraphs
    for (auto& entry : _commandGraphsAndBuffers)
    {
        entry.cg->_disconnect(this);
    }
}

void ExecuteCommands::connect(ref_ptr<SecondaryCommandGraph> commandGraph)
{
    _commandGraphsAndBuffers.push_back(CommandGraphAndBuffer{commandGraph, {}});
    commandGraph->_connect(this);
}

void ExecuteCommands::reset()
{
    std::scoped_lock lock(_mutex);

    _latch->set(static_cast<int>(_commandGraphsAndBuffers.size()));

    for (auto& entry : _commandGraphsAndBuffers)
    {
        entry.cb = {};
    }
}

void ExecuteCommands::completed(const SecondaryCommandGraph& commandGraph, ref_ptr<CommandBuffer> commandBuffer)
{
    if (commandBuffer)
    {
        std::scoped_lock lock(_mutex);

        for (auto& [cg, cb] : _commandGraphsAndBuffers)
        {
            if (cg == &commandGraph)
            {
                cb = commandBuffer;
                break;
            }
        }
    }

    _latch->count_down();
}

void ExecuteCommands::record(CommandBuffer& commandBuffer) const
{
    _latch->wait();

    std::scoped_lock lock(_mutex);
    std::vector<VkCommandBuffer> vk_commandBuffers;
    for (auto& entry : _commandGraphsAndBuffers)
    {
        if (entry.cb) vk_commandBuffers.push_back(*entry.cb);
    }

    if (!vk_commandBuffers.empty())
    {
        vkCmdExecuteCommands(commandBuffer, static_cast<uint32_t>(vk_commandBuffers.size()), vk_commandBuffers.data());
    }
}
