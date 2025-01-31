/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/PushConstants.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

PushConstants::PushConstants() :
    Inherit(2) // slot 0
{
}

PushConstants::PushConstants(VkShaderStageFlags in_stageFlags, uint32_t in_offset, Data* in_data) :
    Inherit(2), // slot 0
    stageFlags(in_stageFlags),
    offset(in_offset),
    data(in_data)
{
}

PushConstants::~PushConstants()
{
}

void PushConstants::read(Input& input)
{
    StateCommand::read(input);

    input.readValue<uint32_t>("stageFlags", stageFlags);
    input.read("offset", offset);
    input.read("data", data);
}

void PushConstants::write(Output& output) const
{
    StateCommand::write(output);

    output.writeValue<uint32_t>("stageFlags", stageFlags);
    output.write("offset", offset);
    output.write("data", data);
}

void PushConstants::record(CommandBuffer& commandBuffer) const
{
    vkCmdPushConstants(commandBuffer, commandBuffer.getCurrentPipelineLayout(), stageFlags, offset, static_cast<uint32_t>(data->dataSize()), data->dataPointer());
}
