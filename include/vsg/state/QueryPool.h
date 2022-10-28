#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Josef Stumpfegger & Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Device.h>

namespace vsg
{
    /// QueryPool encapsulates the VkQueryPool and the VkQueryPoolCreateInfo settings used to set it up.
    class VSG_DECLSPEC QueryPool : public Inherit<Object, QueryPool>
    {
    public:
        QueryPool();

        operator VkQueryPool() const { return _queryPool; }
        VkQueryPool vk() const { return _queryPool; }

        VkQueryPoolCreateFlags flags = 0;
        VkQueryType queryType = VK_QUERY_TYPE_TIMESTAMP;
        uint32_t queryCount = 1;
        VkQueryPipelineStatisticFlags pipelineStatistics = 0;

        void read(Input& input) override;
        void write(Output& output) const override;

        void reset();

        VkResult getResults(std::vector<uint32_t>& results, uint32_t firstQuery = 0, VkQueryResultFlags resultsFlags = VK_QUERY_RESULT_WAIT_BIT) const;
        VkResult getResults(std::vector<uint64_t>& results, uint32_t firstQuery = 0, VkQueryResultFlags resultsFlags = VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT) const;

        void compile(Context& context);

    protected:
        ~QueryPool();

        VkQueryPool _queryPool{};
        ref_ptr<Device> _device{};
    };
    VSG_type_name(vsg::QueryPool);

} // namespace vsg
