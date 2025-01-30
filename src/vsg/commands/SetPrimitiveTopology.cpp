/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/SetPrimitiveTopology.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

SetPrimitiveTopology::SetPrimitiveTopology(VkPrimitiveTopology in_topology) :
    topology(in_topology)
{
}

void SetPrimitiveTopology::read(Input& input)
{
    Command::read(input);

    input.readValue<uint32_t>("topology", topology);
}

void SetPrimitiveTopology::write(Output& output) const
{
    Command::write(output);

    output.writeValue<uint32_t>("topology", topology);
}

void SetPrimitiveTopology::record(CommandBuffer& commandBuffer) const
{
    auto extensions = commandBuffer.getDevice()->getExtensions();
    if (!extensions->vkCmdSetPrimitiveTopology) return;

    extensions->vkCmdSetPrimitiveTopology(commandBuffer, topology);
}
