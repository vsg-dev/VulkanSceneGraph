#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/Latch.h>
#include <vsg/viewer/CommandGraph.h>

namespace vsg
{

    /// Encapsulation of vkCmdExecuteCommands with thread safe integration with secondary CommandGraph that provide the secondary CommandBuffer
    class VSG_DECLSPEC ExecuteCommands : public Inherit<Command, ExecuteCommands>
    {
    public:
        ExecuteCommands();

        /// connect a second CommmandGraph that will provide the CommandBuffer each frame
        void connect(ref_ptr<CommandGraph> commandGraph);

        /// clean the internal cache of CommandBuffer and reset the Latch used to signal when all the connected CommandGraph have completed the recording of their CommandBuffer
        void reset();

        /// called by secondary CommandGraph to pass on the completed CommadnBuffer that the CommandGraph recorded.
        void completed(ref_ptr<CommandBuffer> commandBuffer);

        /// call vkCmdExecuteCommands with all the CommandBuffer that have been recorded with this ExecuteCommands
        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~ExecuteCommands();

        CommandGraphs _commandGraphs;

        ref_ptr<Latch> _latch;

        mutable std::mutex _mutex;
        CommandBuffers _commandBuffers;
    };
    VSG_type_name(vsg::ExecuteCommands);

} // namespace vsg
