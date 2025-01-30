/* <editor-fold desc="MIT License">

Copyright(c) 2022 Josef Stumpfegger & Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/WriteTimestamp.h>

using namespace vsg;

WriteTimestamp::WriteTimestamp()
{
}

WriteTimestamp::WriteTimestamp(VkPipelineStageFlagBits stage, ref_ptr<QueryPool> pool, uint32_t in_query) :
    pipelineStage(stage),
    queryPool(pool),
    query(in_query)
{
}

void WriteTimestamp::read(Input& input)
{
    Command::read(input);

    input.readValue<uint32_t>("pipelineStage", pipelineStage);
    input.readObject("queryPool", queryPool);
    input.read("query", query);
}

void WriteTimestamp::write(Output& output) const
{
    Command::write(output);

    output.writeValue<uint32_t>("pipelineStage", pipelineStage);
    output.writeObject("queryPool", queryPool);
    output.write("query", query);
}

void WriteTimestamp::compile(Context& context)
{
    if (queryPool) queryPool->compile(context);
}

void WriteTimestamp::record(CommandBuffer& commandBuffer) const
{
    if (!queryPool) return;
    vkCmdWriteTimestamp(commandBuffer, pipelineStage, *queryPool, query);
}
