/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/State.h>

void State::reserve(uint32_t maxSlot)
{
    size_t requiredSize = static_cast<size_t>(maxSlot) + 1;
    if (requiredSize > stateStacks.size())
    {
        stateStacks.resize(requiredSize);

        uint32_t numEntries = 1<<requiredSize;

        lookup.resize(numEntries);
        stacks.reserve(numEntries*(requiredSize+1)); // +1 as we need space for the end of list nullptr
        stacks.push_back(nullptr); // 0 mask entry maps to a null/empty list.

        // build up the lookup and stacks vectors to provide the mapping between mask and StateStacks that will be active for them
        for(uint32_t mask = 1; mask < numEntries; ++mask)
        {
            lookup[mask] = &(*stacks.end());
            for(uint32_t slot = 0; slot <= maxSlot; ++slot)
            {
                if ((mask & (1<<slot)) != 0)
                {
                    stacks.push_back(&stateStacks[slot]);
                }
            }
            stacks.push_back(nullptr);
        }
    }
}
