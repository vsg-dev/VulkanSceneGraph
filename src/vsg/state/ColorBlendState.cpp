/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

ColorBlendState::ColorBlendState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_FALSE,                                                                                                 // blendEnable
        VK_BLEND_FACTOR_ZERO,                                                                                     // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                                                                     // dstColorBlendFactor
        VK_BLEND_OP_ADD,                                                                                          // colorBlendOp
        VK_BLEND_FACTOR_ZERO,                                                                                     // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                                                                     // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                                                                          // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // colorWriteMask
    };
    attachments.push_back(colorBlendAttachment);
}

ColorBlendState::ColorBlendState(const ColorBlendState& cbs) :
    Inherit(cbs),
    logicOpEnable(cbs.logicOpEnable),
    logicOp(cbs.logicOp),
    attachments(cbs.attachments),
    blendConstants{cbs.blendConstants[0], cbs.blendConstants[1], cbs.blendConstants[2], cbs.blendConstants[3]}
{
}

ColorBlendState::ColorBlendState(const ColorBlendAttachments& colorBlendAttachments) :
    attachments(colorBlendAttachments)
{
}

ColorBlendState::~ColorBlendState()
{
}

void ColorBlendState::configureAttachments(bool blendEnable)
{
    for (auto& attachment : attachments)
    {
        if (blendEnable)
        {
            attachment.blendEnable = VK_TRUE;
            attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachment.colorBlendOp = VK_BLEND_OP_ADD;
            attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
        else
        {
            attachment.blendEnable = VK_FALSE;
            attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachment.colorBlendOp = VK_BLEND_OP_ADD;
            attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
    }
}

int ColorBlendState::compare(const Object& rhs_object) const
{
    int result = GraphicsPipelineState::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(logicOpEnable, rhs.logicOpEnable))) return result;
    if ((result = compare_value(logicOp, rhs.logicOp))) return result;
    if ((result = compare_value_container(attachments, rhs.attachments))) return result;
    return compare_values(blendConstants, rhs.blendConstants, 3);
}

void ColorBlendState::read(Input& input)
{
    GraphicsPipelineState::read(input);

    input.readValue<uint32_t>("logicOp", logicOp);
    input.readValue<uint32_t>("logicOpEnable", logicOpEnable);

    if (input.version_greater_equal(0, 7, 3))
        attachments.resize(input.readValue<uint32_t>("attachments"));
    else
        attachments.resize(input.readValue<uint32_t>("NumColorBlendAttachments"));

    for (auto& colorBlendAttachment : attachments)
    {
        input.readValue<uint32_t>("blendEnable", colorBlendAttachment.blendEnable);
        input.readValue<uint32_t>("srcColorBlendFactor", colorBlendAttachment.srcColorBlendFactor);
        input.readValue<uint32_t>("dstColorBlendFactor", colorBlendAttachment.dstColorBlendFactor);
        input.readValue<uint32_t>("colorBlendOp", colorBlendAttachment.colorBlendOp);
        input.readValue<uint32_t>("srcAlphaBlendFactor", colorBlendAttachment.srcAlphaBlendFactor);
        input.readValue<uint32_t>("dstAlphaBlendFactor", colorBlendAttachment.dstAlphaBlendFactor);
        input.readValue<uint32_t>("alphaBlendOp", colorBlendAttachment.alphaBlendOp);
        input.readValue<uint32_t>("colorWriteMask", colorBlendAttachment.colorWriteMask);
    }

    input.read("blendConstants", blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);
}

void ColorBlendState::write(Output& output) const
{
    GraphicsPipelineState::write(output);

    output.writeValue<uint32_t>("logicOp", logicOp);
    output.writeValue<uint32_t>("logicOpEnable", logicOpEnable);

    if (output.version_greater_equal(0, 7, 3))
        output.writeValue<uint32_t>("attachments", attachments.size());
    else
        output.writeValue<uint32_t>("NumColorBlendAttachments", attachments.size());

    for (auto& colorBlendAttachment : attachments)
    {
        output.writeValue<uint32_t>("blendEnable", colorBlendAttachment.blendEnable);
        output.writeValue<uint32_t>("srcColorBlendFactor", colorBlendAttachment.srcColorBlendFactor);
        output.writeValue<uint32_t>("dstColorBlendFactor", colorBlendAttachment.dstColorBlendFactor);
        output.writeValue<uint32_t>("colorBlendOp", colorBlendAttachment.colorBlendOp);
        output.writeValue<uint32_t>("srcAlphaBlendFactor", colorBlendAttachment.srcAlphaBlendFactor);
        output.writeValue<uint32_t>("dstAlphaBlendFactor", colorBlendAttachment.dstAlphaBlendFactor);
        output.writeValue<uint32_t>("alphaBlendOp", colorBlendAttachment.alphaBlendOp);
        output.writeValue<uint32_t>("colorWriteMask", colorBlendAttachment.colorWriteMask);
    }

    output.write("blendConstants", blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);
}

void ColorBlendState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto colorBlendState = context.scratchMemory->allocate<VkPipelineColorBlendStateCreateInfo>();

    colorBlendState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState->pNext = nullptr;
    colorBlendState->flags = 0;

    colorBlendState->logicOpEnable = logicOpEnable;
    colorBlendState->logicOp = logicOp;
    colorBlendState->attachmentCount = static_cast<uint32_t>(attachments.size());
    colorBlendState->pAttachments = attachments.data();
    colorBlendState->blendConstants[0] = blendConstants[0];
    colorBlendState->blendConstants[1] = blendConstants[1];
    colorBlendState->blendConstants[2] = blendConstants[2];
    colorBlendState->blendConstants[3] = blendConstants[3];

    pipelineInfo.pColorBlendState = colorBlendState;
}
