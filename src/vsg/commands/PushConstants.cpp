/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PushConstants.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

PushConstants::PushConstants() :
    Inherit(2), // slot 0
    _stageFlags(0),
    _offset(0)
{
}

PushConstants::PushConstants(VkShaderStageFlags stageFlags, uint32_t offset, Data* data) :
    Inherit(2), // slot 0
    _stageFlags(stageFlags),
    _offset(offset),
    _data(data)
{
}

PushConstants::~PushConstants()
{
}

void PushConstants::read(Input& input)
{
    StateCommand::read(input);

    input.readValue<uint32_t>("stageFlags", _stageFlags);
    input.read("offset", _offset);
    input.readObject("data", _data);
}

void PushConstants::write(Output& output) const
{
    StateCommand::write(output);

    output.writeValue<uint32_t>("stageFlags", _stageFlags);
    output.write("offset", _offset);
    output.writeObject("data", _data.get());
}

void PushConstants::record(CommandBuffer& commandBuffer) const
{
    vkCmdPushConstants(commandBuffer, commandBuffer.getCurrentPipelineLayout(), _stageFlags, _offset, static_cast<uint32_t>(_data->dataSize()), _data->dataPointer());
}
