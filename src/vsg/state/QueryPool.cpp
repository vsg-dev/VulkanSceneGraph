/* <editor-fold desc="MIT License">

Copyright(c) 2022 Josef Stumpfegger & Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/state/QueryPool.h>

using namespace vsg;

//////////////////////////////////////
//
// Query Pool
//

QueryPool::QueryPool()
{
}

QueryPool::QueryPool(Device* device, VkQueryPoolCreateFlags in_flags, VkQueryType in_queryType, uint32_t in_queryCount, VkQueryPipelineStatisticFlags in_pipelineStatistics) :
    flags(in_flags),
    queryType(in_queryType),
    queryCount(in_queryCount),
    pipelineStatistics(in_pipelineStatistics)
{
    compile(device);
}

QueryPool::~QueryPool()
{
    if (_queryPool)
        vkDestroyQueryPool(*_device, _queryPool, nullptr);
}

void QueryPool::read(Input& input)
{
    Object::read(input);

    input.readValue<uint32_t>("flags", flags);
    input.readValue<uint32_t>("queryType", queryType);
    input.readValue<uint32_t>("queryCount", queryCount);
    input.readValue<uint32_t>("pipelineStatisticFlags", pipelineStatistics);
}

void QueryPool::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("flags", flags);
    output.writeValue<uint32_t>("queryType", queryType);
    output.writeValue<uint32_t>("queryCount", queryCount);
    output.writeValue<uint32_t>("pipelineStatisticFlags", pipelineStatistics);
}

void QueryPool::reset()
{
    if (!_queryPool) return;

    auto extensions = _device->getExtensions();
    if (extensions->vkResetQueryPool)
    {
        extensions->vkResetQueryPool(*_device, _queryPool, 0, queryCount);
    }
    else
    {
        warn("QueryPool::reset() vkResetQueryPool not supported by device/driver.");
    }
}

VkResult QueryPool::getResults(std::vector<uint32_t>& results, uint32_t firstQuery, VkQueryResultFlags resultsFlags) const
{
    if (!_queryPool) return VK_NOT_READY;
    if (firstQuery > queryCount) return VK_ERROR_UNKNOWN; // out of range

    uint32_t count = std::min(queryCount - firstQuery, static_cast<uint32_t>(results.size()));
    if (count == 0) return VK_ERROR_UNKNOWN; // out of range

    return vkGetQueryPoolResults(*_device, _queryPool, firstQuery, count, count * sizeof(uint32_t), results.data(), sizeof(uint32_t), resultsFlags);
}

VkResult QueryPool::getResults(std::vector<uint64_t>& results, uint32_t firstQuery, VkQueryResultFlags resultsFlags) const
{
    if (!_queryPool) return VK_NOT_READY;
    if (firstQuery > queryCount) return VK_ERROR_UNKNOWN; // out of range

    uint32_t count = std::min(queryCount - firstQuery, static_cast<uint32_t>(results.size()));
    if (count == 0) return VK_ERROR_UNKNOWN; // out of range

    return vkGetQueryPoolResults(*_device, _queryPool, firstQuery, count, count * sizeof(uint64_t), results.data(), sizeof(uint64_t), resultsFlags);
}

void QueryPool::compile(Device* device)
{
    if (_queryPool) return;
    _device = device;
    VkQueryPoolCreateInfo createInfo{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                     {},         //pNext
                                     flags,      //flags
                                     queryType,  //queryType
                                     queryCount, //queryCount
                                     pipelineStatistics};
    if (VkResult res = vkCreateQueryPool(*_device, &createInfo, nullptr, &_queryPool); res != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create QueryPool.", res};
    }
}

void QueryPool::compile(Context& context)
{
    compile(context.device);
}
