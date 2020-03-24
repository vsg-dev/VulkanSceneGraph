#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/CommandGraph.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/State.h>

namespace vsg
{
    /** Execute Secondary Command Buffers
     * (in charge of sync with their filling)
     */

    class VSG_DECLSPEC ExecuteCommands : public Inherit<Command, ExecuteCommands>
    {
    public:
        ExecuteCommands() {}

        using Secondaries = std::vector< ref_ptr < CommandGraph > >;

        //TODO go protected and make other accessors
        Secondaries _cmdGraphs;

        void addCommandGraph(ref_ptr<CommandGraph> d)
        {
            _cmdGraphs.emplace_back( d );
            _commandBuffers.resize(_cmdGraphs.size());
            _mutices.emplace_back(new std::mutex);
        }

        std::mutex * getCommandGraphMutex(const CommandGraph* d) const
        {
            Secondaries::const_iterator iter = std::find(_cmdGraphs.begin(), _cmdGraphs.end(), d);
            size_t index = std::distance(_cmdGraphs.begin(), iter);
            if(index == _cmdGraphs.size())
               return nullptr;
            return _mutices[index].get();
        }
        void read(Input& input) override;
        void write(Output& output) const override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        //cb and mutices to signal secondaries producers that previous produced have been consumed by vkCmdExecuteCommands
        mutable std::vector< VkCommandBuffer > _commandBuffers;
        std::vector< std::unique_ptr<std::mutex> > _mutices;
        virtual ~ExecuteCommands();

    };
    VSG_type_name(vsg::ExecuteCommands);

} // namespace vsg

