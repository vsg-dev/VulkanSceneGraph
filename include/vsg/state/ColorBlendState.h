#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/GraphicsPipeline.h>

namespace vsg
{

    /// ColorBlendState encapsulates to VkPipelineColorBlendStateCreateInfo settings passed when setting up GraphicsPipeline
    class VSG_DECLSPEC ColorBlendState : public Inherit<GraphicsPipelineState, ColorBlendState>
    {
    public:
        using ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>;

        ColorBlendState();
        ColorBlendState(const ColorBlendState& cbs);
        explicit ColorBlendState(const ColorBlendAttachments& colorBlendAttachments);

        /// VkPipelineColorBlendStateCreateInfo settings
        VkBool32 logicOpEnable = VK_FALSE;
        VkLogicOp logicOp = VK_LOGIC_OP_COPY;
        ColorBlendAttachments attachments;
        float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~ColorBlendState();
    };
    VSG_type_name(vsg::ColorBlendState);

} // namespace vsg
