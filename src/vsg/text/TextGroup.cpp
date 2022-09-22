/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/TextGroup.h>
#include <vsg/utils/GraphicsPipelineConfig.h>
#include <vsg/utils/SharedObjects.h>


using namespace vsg;

int TextGroup::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer_container(children, rhs.children);
}

void TextGroup::read(Input& input)
{
    Node::read(input);

    input.readObjects("children", children);
}

void TextGroup::write(Output& output) const
{
    Node::write(output);

    output.writeObjects("children", children);
}

void TextGroup::addChild(ref_ptr<Text> text)
{
    if (children.empty())
    {
        if (!text->technique) text->technique = CpuLayoutTechnique::create();
        children.push_back(text);
    }
    else
    {
        // technique, font and shaderSet are taken from the fist entry
        text->technique = {};
        text->font = {};
        text->shaderSet = {};

        children.push_back(text);
    }
}

void TextGroup::setup(uint32_t minimumAllocation)
{
    if (children.empty()) return;

    auto& first_text = children.front();
    auto& technique = first_text->technique;
    technique->setup(this, minimumAllocation);
}
