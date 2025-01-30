/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/ClearAttachments.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

ClearAttachments::ClearAttachments()
{
}

ClearAttachments::ClearAttachments(const Attachments& in_attachments, const Rects& in_rects) :
    attachments(in_attachments),
    rects(in_rects)
{
}

void ClearAttachments::read(Input& input)
{
    Command::read(input);

    attachments.resize(input.readValue<uint32_t>("attachments"));
    for (auto& attachment : attachments)
    {
        input.readValue<uint32_t>("aspectMask", attachment.aspectMask);
        input.read("colorAttachment", attachment.colorAttachment);
        input.read("clearValue", attachment.clearValue);
    }

    rects.resize(input.readValue<uint32_t>("rects"));
    for (auto& r : rects)
    {
        input.read("rect", r.rect.offset.x, r.rect.offset.y, r.rect.extent.width, r.rect.extent.height);
        input.read("baseArrayLayer", r.baseArrayLayer);
        input.read("layerCount", r.layerCount);
    }
}

void ClearAttachments::write(Output& output) const
{
    Command::write(output);

    output.writeValue<uint32_t>("attachments", attachments.size());
    for (auto& attachment : attachments)
    {
        output.writeValue<uint32_t>("aspectMask", attachment.aspectMask);
        output.write("colorAttachment", attachment.colorAttachment);
        output.write("clearValue", attachment.clearValue);
    }

    output.writeValue<uint32_t>("rects", rects.size());
    for (auto& r : rects)
    {
        output.write("rect", r.rect.offset.x, r.rect.offset.y, r.rect.extent.width, r.rect.extent.height);
        output.write("baseArrayLayer", r.baseArrayLayer);
        output.write("layerCount", r.layerCount);
    }
}

void ClearAttachments::record(CommandBuffer& commandBuffer) const
{
    vkCmdClearAttachments(commandBuffer,
                          static_cast<uint32_t>(attachments.size()), attachments.data(),
                          static_cast<uint32_t>(rects.size()), rects.data());
}
