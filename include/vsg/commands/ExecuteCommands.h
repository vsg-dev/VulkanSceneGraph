#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/SecondaryCommandGraph.h>
#include <vsg/threading/Latch.h>

namespace vsg
{

    /// Encapsulation of vkCmdExecuteCommands with thread safe integration with secondary CommandGraph that provide the secondary CommandBuffer
    class VSG_DECLSPEC ExecuteCommands : public Inherit<Command, ExecuteCommands>
    {
    public:
        ExecuteCommands();

        /// connect a SecodaryCommmandGraph that will provide the CommandBuffer each frame
        void connect(ref_ptr<SecondaryCommandGraph> commandGraph);

        /// clean the internal cache of CommandBuffer and reset the Latch used to signal when all the connected CommandGraph have completed the recording of their CommandBuffer
        void reset();

        /// called by secondary CommandGraph to pass on the completed CommandBuffer that the CommandGraph recorded.
        void completed(const SecondaryCommandGraph& commandGraph, ref_ptr<CommandBuffer> commandBuffer);

        /// call vkCmdExecuteCommands with all the CommandBuffer that have been recorded with this ExecuteCommands
        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~ExecuteCommands();

        struct CommandGraphAndBuffer
        {
            ref_ptr<SecondaryCommandGraph> cg;
            ref_ptr<CommandBuffer> cb;
        };

        mutable std::mutex _mutex;
        ref_ptr<Latch> _latch;

        std::vector<CommandGraphAndBuffer> _commandGraphsAndBuffers;
    };
    VSG_type_name(vsg::ExecuteCommands);

} // namespace vsg
