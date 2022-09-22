/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array2D.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/io/Logger.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/text/CpuLayoutTechnique.h>
#include <vsg/text/GpuLayoutTechnique.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

#include "shaders/text_frag.cpp"
#include "shaders/text_vert.cpp"

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

    setup();
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

void Text::setup(uint32_t minimumAllocation)
{
    if (!layout) layout = StandardLayout::create();
    if (!technique) technique = CpuLayoutTechnique::create();

    technique->setup(this, minimumAllocation);
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

    // load shaders
    auto vertexShader = read_cast<ShaderStage>("shaders/text.vert", options);
    if (!vertexShader) vertexShader = text_vert(); // fallback to shaders/text_vert.cpp

    auto fragmentShader = read_cast<ShaderStage>("shaders/text.frag", options);
    if (!fragmentShader) fragmentShader = text_frag(); // fallback to shaders/text_frag.cpp

    uint32_t numTextIndices = 256;
    vertexShader->specializationConstants = vsg::ShaderStage::SpecializationConstants{
        {0, vsg::uintValue::create(numTextIndices)} // numTextIndices
    };

    auto shaderSet = ShaderSet::create(ShaderStages{vertexShader, fragmentShader});

    // used for both CPU and GPU layouts
    shaderSet->addAttributeBinding("inPosition", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vec3Array::create(1));
    shaderSet->addUniformBinding("textureAtlas", "", 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vec4Array2D::create(1, 1));
    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    // only used when using CPU Layout
    shaderSet->addAttributeBinding("inColor", "CPU_LAYOUT", 1, VK_FORMAT_R32G32B32A32_SFLOAT, vec4Array::create(1));
    shaderSet->addAttributeBinding("inOutlineColor", "CPU_LAYOUT", 2, VK_FORMAT_R32G32B32A32_SFLOAT, vec4Array::create(1));
    shaderSet->addAttributeBinding("inOutlineWidth", "CPU_LAYOUT", 3, VK_FORMAT_R32_SFLOAT, floatArray::create(1));
    shaderSet->addAttributeBinding("inTexCoord", "CPU_LAYOUT", 4, VK_FORMAT_R32G32B32_SFLOAT, vec3Array::create(1));
    shaderSet->addAttributeBinding("inCenterAndAutoScaleDistance", "BILLBOARD", 5, VK_FORMAT_R32G32B32A32_SFLOAT, vec4Array::create(1));

    // only used when using GPU Layout
    shaderSet->addUniformBinding("glyphMetrics", "GPU_LAYOUT", 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vec4Array2D::create(1, 1));
    shaderSet->addUniformBinding("textLayout", "GPU_LAYOUT", 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, TextLayoutValue::create());
    shaderSet->addUniformBinding("text", "GPU_LAYOUT", 1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, uivec4Array2D::create(1, 1));

    return shaderSet;
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
