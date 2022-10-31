#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/sphere.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// Bin node is used internally by RecoredTraversal/View to collect and then sort command nodes assigned the bin,
    /// then recorded to the command buffer in the sorted order.
    class VSG_DECLSPEC Bin : public Inherit<Node, Bin>
    {
    public:
        enum SortOrder
        {
            NO_SORT,
            ASCENDING,
            DESCENDING
        };

        Bin();
        Bin(int32_t in_binNumber, SortOrder in_sortOrder);

        void traverse(RecordTraversal& visitor) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void clear();

        void add(State* state, double value, const Node* node);

        int32_t binNumber = 0;
        SortOrder sortOrder = NO_SORT;

    protected:
        virtual ~Bin();

        std::vector<dmat4> _matrices;
        std::vector<const StateCommand*> _stateCommands;

        struct Element
        {
            uint32_t matrixIndex = 0;
            uint32_t stateCommandIndex = 0;
            uint32_t stateCommandCount = 0;
            const Node* child = nullptr;
        };

        std::vector<Element> _elements;

        using KeyIndex = std::pair<float, uint32_t>;
        mutable std::vector<KeyIndex> _binElements;
    };
    VSG_type_name(vsg::Bin);

} // namespace vsg
