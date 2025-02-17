/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/nodes/Bin.h>
#include <vsg/vk/State.h>

#include <algorithm>

using namespace vsg;

Bin::Bin()
{
}

Bin::Bin(const Bin& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    binNumber(rhs.binNumber),
    sortOrder(rhs.sortOrder)
{
}

Bin::Bin(int32_t in_binNumber, SortOrder in_sortOrder) :
    binNumber(in_binNumber),
    sortOrder(in_sortOrder)
{
}

Bin::~Bin()
{
}

int Bin::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(binNumber, rhs.binNumber)) != 0) return result;
    return compare_value(sortOrder, rhs.sortOrder);
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
    //debug("Bin::add(state= ", state, ", value = ", value, ", ", node, ") ", this, ", binNumber = ", binNumber, ",  binElements.size()=", _binElements.size());

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
            //debug("reoccurring ");
            element.matrixIndex = static_cast<uint32_t>(_matrices.size()) - 1;
        }
        else
        {
            //debug("new ");
            element.matrixIndex = static_cast<uint32_t>(_matrices.size());
            _matrices.push_back(mv);
        }
    }
#else
    element.matrixIndex = _matrices.size();
    _matrices.push_back(mv);
#endif

    element.stateCommandIndex = static_cast<uint32_t>(_stateCommands.size());

#if 0
    for (const State::StateCommandStack* last_dirty = state->last_dirty; last_dirty != nullptr; last_dirty = last_dirty->last_dirty)
    {
        _stateCommands.push_back(last_dirty->top());
        ++element.stateCommandCount;
    }
#else
    for (auto& stateStack : state->stateStacks)
    {
        if (stateStack.size() > 0)
        {
            _stateCommands.push_back(stateStack.top());
            ++element.stateCommandCount;
        }
    }
#endif

    element.child = node;

    _binElements.emplace_back(static_cast<float>(value), static_cast<uint32_t>(_elements.size()));

    _elements.push_back(element);
}

void Bin::traverse(RecordTraversal& rt) const
{
    //debug("Bin::traverse(RecordTraversal& visitor) ", sortOrder, " ", _binElements.size());

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

    for (const auto& keyElement : _binElements)
    {
        const auto& element = _elements[keyElement.second];

        if (element.matrixIndex != previousMatrixIndex)
        {
            state->modelviewMatrixStack.push(_matrices[element.matrixIndex]);
            state->applyFrustum();
            state->dirty = true;
            previousMatrixIndex = element.matrixIndex;
            //debug("    updating");
        }
        else
        {
            //debug("    No need to update");
        }

        if (element.stateCommandCount > 0)
        {
            auto begin = _stateCommands.begin() + element.stateCommandIndex;
            auto end = begin + element.stateCommandCount;
            state->push(begin, end);

            element.child->accept(rt);

            state->pop(begin, end);
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

    output.write("binNumber", binNumber);
    output.write("sortOrder", sortOrder);
}
