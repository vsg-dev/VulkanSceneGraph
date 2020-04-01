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
     * (in charge of blocking-sync- their filling)
     */

    class VSG_DECLSPEC ExecuteCommands : public Inherit<Command, ExecuteCommands>
    {
    public:
        ExecuteCommands() {}
        struct SecondaryGraph
        {
            SecondaryGraph(ref_ptr < CommandGraph >& cg):
                commandGraph(cg), consumptionMutex(new std::mutex), productionMutex(nullptr) {}
            ref_ptr < CommandGraph > commandGraph;
            std::unique_ptr<std::mutex> consumptionMutex;
            std::shared_ptr<std::mutex> productionMutex; //setup in Viewer
        };
        using Secondaries = std::vector< SecondaryGraph >;

        Secondaries & getSecondaryCommandGraphs() { return _cmdGraphs; }
        const Secondaries & getSecondaryCommandGraphs() const { return _cmdGraphs; }

        void addCommandGraph(ref_ptr<CommandGraph> d)
        {
            _cmdGraphs.emplace_back(SecondaryGraph(d));
            _commandBuffers.resize(_cmdGraphs.size());
        }

        void read(Input& input) override;
        void write(Output& output) const override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        Secondaries _cmdGraphs;
        //cb and mutices to signal secondaries producers that previous produced have been consumed by vkCmdExecuteCommands
        mutable std::vector< VkCommandBuffer > _commandBuffers;
        virtual ~ExecuteCommands();

    };
    VSG_type_name(vsg::ExecuteCommands);

} // namespace vsg

