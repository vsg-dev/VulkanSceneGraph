/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/nodes/Bin.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

Bin::Bin(Allocator* allocator) :
    Inherit(allocator)
{
}

Bin::Bin(uint32_t in_binNumber, SortOrder in_sortOrder, Allocator* allocator) :
    Inherit(allocator),
    binNumber(in_binNumber),
    sortOrder(in_sortOrder)
{
}

Bin::~Bin()
{
}

void Bin::add(double value, const Node* node)
{
    binElements.emplace_back(value, node);

    std::cout<<"Bin::add("<<value<<", "<<node<<") "<<this<<", binNumber = "<<binNumber<<",  binElements.size()="<<binElements.size()<<std::endl;
}

void Bin::traverse(RecordTraversal& visitor) const
{
    std::cout<<"Bin::traverse(RecordTraversal& visitor) "<<sortOrder<<" "<<binElements.size()<<std::endl;

    switch(sortOrder)
    {
        case(ASCENDING):
            std::sort(binElements.begin(), binElements.end(), [](const KeyNode& lhs, const KeyNode& rhs) { return lhs.first < rhs.first; });
            break;
        case(DESCENDING):
            std::sort(binElements.begin(), binElements.end(), [](const KeyNode& lhs, const KeyNode& rhs) { return rhs.first < lhs.first; });
            break;
        case(NO_SORT):
            break;
    }

    for(auto& keyNode : binElements)
    {
        std::cout<<"   "<<keyNode.first<<" "<<keyNode.second->className()<<std::endl;
        keyNode.second->accept(visitor);
    }
}

void Bin::read(Input& input)
{
    Node::read(input);

    input.read("binNumber", binNumber);
    input.read("sortOrder", sortOrder);
}

void Bin::write(Output& output) const
{
    Node::write(output);

    output.write("binNimber", binNumber);
    output.write("sortOrder", sortOrder);
}

