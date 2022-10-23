/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array2D.h>
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

#include "shaders/text_ShaderSet.cpp"

using namespace vsg;

void Text::read(Input& input)
{
    Node::read(input);

    input.readObject("font", font);

    if (input.version_greater_equal(0, 5, 2))
    {
        input.readObject("shaderSet", shaderSet);
    }

    input.readObject("technique", technique);
    input.readObject("layout", layout);
    input.readObject("text", text);

    setup(0, input.options);
}

void Text::write(Output& output) const
{
    Node::write(output);

    output.writeObject("font", font);

    if (output.version_greater_equal(0, 5, 2))
    {
        output.writeObject("shaderSet", shaderSet);
    }

    output.writeObject("technique", technique);
    output.writeObject("layout", layout);
    output.writeObject("text", text);
}

void Text::setup(uint32_t minimumAllocation, ref_ptr<const Options> options)
{
    if (!layout) layout = StandardLayout::create();
    if (!technique) technique = CpuLayoutTechnique::create();

    technique->setup(this, minimumAllocation, options);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// createTextShaderSet
//
ref_ptr<ShaderSet> vsg::createTextShaderSet(ref_ptr<const Options> options)
{
    if (options)
    {
        // check if a ShaderSet has already been assigned to the options object, if so return it
        if (auto itr = options->shaderSets.find("text"); itr != options->shaderSets.end()) return itr->second;
    }

    return text_ShaderSet();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CountGlyphs
//
void CountGlyphs::apply(const stringValue& text)
{
    count += text.value().size();
}

void CountGlyphs::apply(const ubyteArray& text)
{
    count += text.size();
}

void CountGlyphs::apply(const ushortArray& text)
{
    count += text.size();
}

void CountGlyphs::apply(const uintArray& text)
{
    count += text.size();
}
