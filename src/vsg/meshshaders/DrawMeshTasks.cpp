/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/meshshaders/DrawMeshTasks.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

DrawMeshTasks::DrawMeshTasks()
{
}

DrawMeshTasks::DrawMeshTasks(uint32_t in_groupCountX, uint32_t in_groupCountY, uint32_t in_groupCountZ) :
    groupCountX(in_groupCountX),
    groupCountY(in_groupCountY),
    groupCountZ(in_groupCountZ)
{
}

void DrawMeshTasks::read(Input& input)
{
    input.read("groupCountX", groupCountX);
    input.read("groupCountY", groupCountY);
    input.read("groupCountZ", groupCountZ);
}

void DrawMeshTasks::write(Output& output) const
{
    output.write("groupCountX", groupCountX);
    output.write("groupCountY", groupCountY);
    output.write("groupCountZ", groupCountZ);
}

void DrawMeshTasks::record(vsg::CommandBuffer& commandBuffer) const
{
    Device* device = commandBuffer.getDevice();
    auto extensions = device->getExtensions();
    extensions->vkCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
