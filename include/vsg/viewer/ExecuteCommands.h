#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

//#include <vsg/nodes/StateGroup.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/State.h>

namespace vsg
{

    /** Execute Secondary Command Buffers
     * (in charge of blocking-sync- their filling?)
    }*/

    class VSG_DECLSPEC ExecuteCommands : public Inherit<Command, ExecuteCommands>
    {
    public:
        ExecuteCommands() {}

      /*  ExecuteCommands(Data* indices);

        ExecuteCommands(const BufferData& bufferData);

        ExecuteCommands(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType) :
            _bufferData(buffer, offset, 0),
            _indexType(indexType) {}
*/
        using StateGroups = std::vector< ref_ptr<StateGroup> >;
        StateGroups _secondarygraphs;
        ///

        std::vector<ref_ptr<CommandBuffer> > records;
        using Secondaries = std::vector< ref_ptr < CommandGraph > >;
        Secondaries _cmdgraphs;

        mutable std::vector< VkCommandBuffer > _commandbuffers;
        void addCommandGraph(ref_ptr<CommandGraph> d) { _cmdgraphs.emplace_back(d); //addChild(d);
                                                      }

     /*
        void setIndices(ref_ptr<Data> indices) { _bufferData._data = indices; }
        Data* getIndices() { return _bufferData._data; }
        const Data* getIndices() const { return _bufferData._data; }*/

        void read(Input& input) override;
        void write(Output& output) const override;
    void compile(Context& context) override;

        void dispatch(CommandBuffer& commandBuffer) const override;

     /*
      //   void accept(Visitor& visitor) override { visitor.apply(static_cast<Subclass&>(*this)); }
void accept(RecordTraversal& dispatchTraversal) const override;
        void traverse(Visitor& visitor) override;
        void traverse(ConstVisitor& visitor) const override ;
        void traverse(RecordTraversal& visitor) const override ;
        void traverse(CullTraversal& visitor) const override;
*/
    protected:
        virtual ~ExecuteCommands();

        BufferData _bufferData;
        VkIndexType _indexType;
    };
    VSG_type_name(vsg::ExecuteCommands);

} // namespace vsg
