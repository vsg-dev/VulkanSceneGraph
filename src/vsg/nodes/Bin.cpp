/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/nodes/Bin.h>
#include <vsg/vk/State.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

Bin::Bin(Allocator* allocator) :
    Inherit(allocator)
{
}

Bin::Bin(int32_t in_binNumber, SortOrder in_sortOrder, Allocator* allocator) :
    Inherit(allocator),
    binNumber(in_binNumber),
    sortOrder(in_sortOrder)
{
}

Bin::~Bin()
{
}

void Bin::clear()
{
    _matrices.clear();
    _stateCommands.clear();
    _elements.clear();
    _binElements.clear();
}

void Bin::add(State* state, double value, const Node* node)
{
    //    binElements.emplace_back(value, node);

    //std::cout<<"Bin::add(state= "<<state<<", value = "<<value<<", "<<node<<") "<<this<<", binNumber = "<<binNumber<<",  binElements.size()="<<_binElements.size()<<std::endl;

    Element element;

    const auto& mv = state->modelviewMatrixStack.top();
#if 1
    if (_matrices.empty())
    {
        element.matrixIndex = static_cast<uint32_t>(_matrices.size());
        _matrices.push_back(mv);
    }
    else
    {
        if (_matrices.back() == mv)
        {
            //std::cout<<"reoccurring "<<std::endl;
            element.matrixIndex = static_cast<uint32_t>(_matrices.size()) - 1;
        }
        else
        {
            //std::cout<<"new "<<std::endl;
            element.matrixIndex = static_cast<uint32_t>(_matrices.size());
            _matrices.push_back(mv);
        }
    }
#else
    element.matrixIndex = _matrices.size();
    _matrices.push_back(mv);
#endif

    element.stateCommandIndex = static_cast<uint32_t>(_stateCommands.size());
    for (auto& stateStack : state->stateStacks)
    {
        if (stateStack.size() > 0)
        {
            _stateCommands.push_back(stateStack.top());
            ++element.stateCommandCount;
        }
    }

    element.child = node;

    _binElements.emplace_back(static_cast<float>(value), static_cast<uint32_t>(_elements.size()));

    _elements.push_back(element);
}

void Bin::traverse(RecordTraversal& rt) const
{
    //std::cout<<"Bin::traverse(RecordTraversal& visitor) "<<sortOrder<<" "<<_binElements.size()<<std::endl;

    auto state = rt.getState();

    switch (sortOrder)
    {
    case (ASCENDING):
        std::sort(_binElements.begin(), _binElements.end(), [](const KeyIndex& lhs, const KeyIndex& rhs) { return lhs.first < rhs.first; });
        break;
    case (DESCENDING):
        std::sort(_binElements.begin(), _binElements.end(), [](const KeyIndex& lhs, const KeyIndex& rhs) { return rhs.first < lhs.first; });
        break;
    case (NO_SORT):
        break;
    }

    uint32_t previousMatrixIndex = static_cast<uint32_t>(_matrices.size());
    //uint32_t previousStateCommandIndex = _stateCommands.size();

    state->pushFrustum();
    state->dirty = true;

    for (auto& keyElement : _binElements)
    {
        //std::cout<<"   "<<keyNode.first<<" "<<keyNode.second->className()<<std::endl;
        auto& element = _elements[keyElement.second];

        if (element.matrixIndex != previousMatrixIndex)
        {
            state->modelviewMatrixStack.push(_matrices[element.matrixIndex]);
            state->applyFrustum();
            state->dirty = true;
            previousMatrixIndex = element.matrixIndex;
            //std::cout<<"updating"<<std::endl;
        }
        else
        {
            //std::cout<<"No need to update"<<std::endl;
        }

        if (element.stateCommandCount > 0)
        {
            uint32_t endIndex = element.stateCommandIndex + element.stateCommandCount;
            for (uint32_t i = element.stateCommandIndex; i < endIndex; ++i)
            {
                auto command = _stateCommands[i];
                state->stateStacks[command->slot].push(command);
            }

            element.child->accept(rt);

            for (uint32_t i = element.stateCommandIndex; i < endIndex; ++i)
            {
                auto command = _stateCommands[i];
                state->stateStacks[command->slot].pop();
            }
        }
        else
        {
            element.child->accept(rt);
        }
    }

    state->popFrustum();
    state->dirty = true;
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
