/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/viewer/ExecuteCommands.h>

using namespace vsg;

ExecuteCommands::ExecuteCommands()
{
}

ExecuteCommands::~ExecuteCommands()
{
    // disconnect all the CommandGraphs
    for (auto& commandGraph : _commandGraphs)
    {
        commandGraph->_disconnect(this);
    }
}

void ExecuteCommands::connect(ref_ptr<CommandGraph> commandGraph)
{
    _commandGraphs.emplace_back(commandGraph);
    commandGraph->_connect(this);
}

void ExecuteCommands::reset()
{
    std::scoped_lock lock(_mutex);

    if (!_latch)
        _latch = vsg::Latch::create(static_cast<int>(_commandGraphs.size()));
    else
        _latch->set(static_cast<int>(_commandGraphs.size()));

    _commandBuffers.clear();
}

void ExecuteCommands::completed(ref_ptr<CommandBuffer> commandBuffer)
{
    if (commandBuffer)
    {
        std::scoped_lock lock(_mutex);
        _commandBuffers.emplace_back(commandBuffer);
    }

    _latch->count_down();
}

void ExecuteCommands::record(CommandBuffer& commandBuffer) const
{
    _latch->wait();

    std::scoped_lock lock(_mutex);
    if (!_commandBuffers.empty())
    {
        std::vector<VkCommandBuffer> vk_commandBuffers;

        for (auto& cb : _commandBuffers)
        {
            vk_commandBuffers.push_back(*cb);
        }

        vkCmdExecuteCommands(commandBuffer, static_cast<uint32_t>(vk_commandBuffers.size()), vk_commandBuffers.data());
    }
}
