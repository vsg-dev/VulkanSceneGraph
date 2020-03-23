/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/ApplicationEvent.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/vk/ExecuteCommands.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;
ExecuteCommands::~ExecuteCommands()
{
}

void ExecuteCommands::read(Input& input)
{
    Command::read(input);

    // read secondary command graphs
    //_cmdgraphs = input.readObject<CommandGraphs>("CommandGraphs");
}

void ExecuteCommands::write(Output& output) const
{
    Command::write(output);

    // write secondary command graphs
    // output.writeObject("CommandGraphs", _cmdgraphs);
}


void ExecuteCommands::dispatch(CommandBuffer& commandBuffer) const
{
    _commandBuffers.clear();

    for(auto cg : _cmdGraphs)
    {
        cg->waitProduction();
        _commandBuffers.emplace_back(*cg->lastRecorded);
    }

    vkCmdExecuteCommands(commandBuffer, _commandBuffers.size(), _commandBuffers.data());

    //unlock producers
    for(auto mit = _mutices.begin(); mit != _mutices.end(); ++mit)
        (*mit)->unlock();
}

