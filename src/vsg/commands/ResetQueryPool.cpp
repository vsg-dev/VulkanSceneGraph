/* <editor-fold desc="MIT License">

Copyright(c) 2022 Josef Stumpfegger & Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/ResetQueryPool.h>

using namespace vsg;

ResetQueryPool::ResetQueryPool()
{
}

ResetQueryPool::ResetQueryPool(ref_ptr<QueryPool> pool) :
    queryPool(pool),
    firstQuery(0),
    queryCount(pool->queryCount)
{
}

void ResetQueryPool::read(Input& input)
{
    Command::read(input);

    input.readObject("queryPool", queryPool);
    input.read("firstQuery", firstQuery);
    input.read("queryCount", queryCount);
}

void ResetQueryPool::write(Output& output) const
{
    Command::write(output);

    output.writeObject("queryPool", queryPool);
    output.write("firstQuery", firstQuery);
    output.write("queryCount", queryCount);
}

void ResetQueryPool::compile(Context& context)
{
    if (queryPool) queryPool->compile(context);
}

void ResetQueryPool::record(CommandBuffer& commandBuffer) const
{
    if (!queryPool) return;
    vkCmdResetQueryPool(commandBuffer, *queryPool, firstQuery, queryCount);
}
