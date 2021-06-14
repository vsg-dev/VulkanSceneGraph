/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

InputAssemblyState::InputAssemblyState()
{
}

InputAssemblyState::InputAssemblyState(VkPrimitiveTopology primitiveTopology, VkBool32 primitiveRestart) :
    topology(primitiveTopology),
    primitiveRestartEnable(primitiveRestart)
{
}

InputAssemblyState::~InputAssemblyState()
{
}

void InputAssemblyState::read(Input& input)
{
    Object::read(input);

    input.readValue<uint32_t>("topology", topology);
    primitiveRestartEnable = input.readValue<uint32_t>("primitiveRestartEnable") != 0;
}

void InputAssemblyState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("topology", topology);
    output.writeValue<uint32_t>("primitiveRestartEnable", primitiveRestartEnable ? 1 : 0);
}

void InputAssemblyState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto inputAssemblyState = context.scratchMemory->allocate<VkPipelineInputAssemblyStateCreateInfo>();

    inputAssemblyState->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState->pNext = nullptr;
    inputAssemblyState->flags = 0;
    inputAssemblyState->topology = topology;
    inputAssemblyState->primitiveRestartEnable = primitiveRestartEnable;

    pipelineInfo.pInputAssemblyState = inputAssemblyState;
}
