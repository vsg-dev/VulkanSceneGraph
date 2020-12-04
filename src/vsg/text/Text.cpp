/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array2D.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

#include <iostream>

using namespace vsg;

void Text::read(Input& input)
{
    Node::read(input);

    input.readObject("font", font);
    input.readObject("technique", technique);
    input.readObject("layout", layout);
    input.readObject("text", text);

    setup();
}

void Text::write(Output& output) const
{
    Node::write(output);

    output.writeObject("font", font);
    output.writeObject("technique", technique);
    output.writeObject("layout", layout);
    output.writeObject("text", text);
}

void Text::setup(uint32_t minimumAllocation)
{
    if (!layout) layout = StandardLayout::create();
    if (!technique) technique = CpuLayoutTechnique::create();

    technique->setup(this, minimumAllocation);
}
