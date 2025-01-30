/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/nodes/PagedLOD.h>

using namespace vsg;

#define PRINT_CONTAINER 0
#define CHECK_CONTAINER 0

//static std::atomic_uint s_numPagedLODS{0};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// PagedLOD
//
PagedLOD::PagedLOD()
{
    //    ++s_numPagedLODS;
}

PagedLOD::PagedLOD(const PagedLOD& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    filename(rhs.filename),
    bound(rhs.bound)
{
    children[0].minimumScreenHeightRatio = rhs.children[0].minimumScreenHeightRatio;
    children[0].node = copyop(rhs.children[0].node);
    children[1].minimumScreenHeightRatio = rhs.children[1].minimumScreenHeightRatio;
    children[1].node = copyop(rhs.children[1].node);
}

PagedLOD::~PagedLOD()
{
    //    --s_numPagedLODS;
    //    vsg::debug("s_numPagedLODS = ", s_numPagedLODS);
}

int PagedLOD::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(bound, rhs.bound)) != 0) return result;

    // compare the children vector
    auto rhs_itr = rhs.children.begin();
    for (auto lhs_itr = children.begin(); lhs_itr != children.end(); ++lhs_itr, ++rhs_itr)
    {
        if ((result = compare_value(lhs_itr->minimumScreenHeightRatio, rhs_itr->minimumScreenHeightRatio)) != 0) return result;
        if ((result = compare_pointer(lhs_itr->node, rhs_itr->node)) != 0) return result;
    }

    return compare_value(filename, rhs.filename);
}

void PagedLOD::read(Input& input)
{
    Node::read(input);

    input.read("bound", bound);

    input.read("child.minimumScreenHeightRatio", children[0].minimumScreenHeightRatio);
    input.read("child.filename", filename);
    children[0].node = nullptr;

    if (input.filename)
    {
        auto path = filePath(input.filename);
        if (path)
        {
            filename = (path / filename).lexically_normal();
        }
    }

    input.read("child.minimumScreenHeightRatio", children[1].minimumScreenHeightRatio);
    input.read("child.node", children[1].node);

    options = Options::create_if(input.options, *input.options);
}

void PagedLOD::write(Output& output) const
{
    Node::write(output);

    output.write("bound", bound);

    output.write("child.minimumScreenHeightRatio", children[0].minimumScreenHeightRatio);
    output.write("child.filename", filename);

    output.write("child.minimumScreenHeightRatio", children[1].minimumScreenHeightRatio);
    output.write("child.node", children[1].node);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// PagedLODContainer
//
PagedLODContainer::PagedLODContainer(uint32_t maxNumPagedLOD) :
    elements(1)
{
    availableList.name = "availableList";
    activeList.name = "activeList";
    inactiveList.name = "inactiveList";

    resize(std::max(maxNumPagedLOD, 10u));
}

void PagedLODContainer::resize(uint32_t new_size)
{
    // note first entry in elements is the null entry, so have to add/take away 1 when accounting for it.
    uint32_t original_size = static_cast<uint32_t>(elements.size()) - 1;
    elements.resize(new_size + 1);

    uint32_t i = 1 + original_size;
    uint32_t previous = availableList.tail;

    if (availableList.head == 0)
    {
        availableList.head = i;
    }

    if (availableList.tail > 0)
    {
        elements[availableList.tail].next = i;
    }

    for (; i < new_size; ++i)
    {
        auto& element = elements[i];
        element.previous = previous;
        element.next = i + 1;
        element.list = &availableList;
        previous = i;
    }

    // set up tail
    elements[i].previous = previous;
    elements[i].next = 0;
    elements[i].list = &availableList;

    availableList.tail = i;

    availableList.count += (new_size - original_size);

#if PRINT_CONTAINER
    debug("PagedLODContainer::resize(", new_size, ")");
    debug_stream([&](auto& fout) { print(fout); });
#endif
}

void PagedLODContainer::resize()
{
    uint32_t original_size = static_cast<uint32_t>(elements.size() - 1);
    uint32_t new_size = original_size * 2;
    resize(new_size);
}

void PagedLODContainer::print(std::ostream& fout)
{
    uint32_t total_size = static_cast<uint32_t>(elements.size());
    fout << "    PagedLODContainer::print() elements.size() = " << total_size << std::endl;
    fout << "        availableList, " << &availableList << ", head  = " << availableList.head << ", tail = " << availableList.tail << " count = " << availableList.count << std::endl;
    fout << "        activeList, " << &activeList << ", head  = " << activeList.head << ", tail = " << activeList.tail << " count = " << activeList.count << std::endl;
    fout << "        inactiveList = " << &inactiveList << ", head  = " << inactiveList.head << ", tail = " << inactiveList.tail << " count = " << inactiveList.count << std::endl;

    for (unsigned i = 0; i < total_size; ++i)
    {
        const auto& element = elements[i];
        fout << "         element[" << i << "] plod = " << element.plod.get() << ", previous =" << element.previous << ", next = " << element.next << ", list = ";
        if (element.list)
            fout << element.list->name;
        else
            fout << " unassigned";
        fout << std::endl;
    }
}

void PagedLODContainer::_move(const PagedLOD* plod, List* targetList)
{
    if (plod->index == 0)
    {
#if PRINT_CONTAINER
        debug("plod not yet assigned, assigning to ", targetList->name);
#endif
        // resize if there are no available empty elements.
        if (availableList.head == 0)
        {
            resize();
        }

        // take the first element from availableList and move head to next item.
        uint32_t index = availableList.head;
        auto& element = elements[availableList.head];
        if (availableList.head == availableList.tail)
        {
            availableList.head = 0;
            availableList.tail = 0;
        }
        else
        {
            availableList.head = element.next;
        }

        if (element.next > 0)
        {
            auto& next_element = elements[element.next];
            next_element.previous = 0;
        }

        // place element at the end of the active list.
        if (targetList->tail > 0)
        {
            auto& previous_element = elements[targetList->tail];
            previous_element.next = index;
        }

        if (targetList->head == 0)
        {
            targetList->head = index;
        }

        element.previous = targetList->tail;
        element.next = 0;
        element.list = targetList;
        targetList->tail = index;

        // assign index to PagedLOD
        plod->index = index;
        element.plod = const_cast<PagedLOD*>(plod);

        --(availableList.count);
        ++(targetList->count);

        return;
    }

    auto& element = elements[plod->index];
    List* previousList = element.list;

    if (previousList == targetList)
    {
#if PRINT_CONTAINER
        debug("PagedLODContainer::move(", plod, ") index = ", plod->index, ", already in ", targetList->name);
#endif
        return;
    }

#if PRINT_CONTAINER
    debug("PagedLODContainer::move(", plod, ") index = ", plod->index, ", moving from ", previousList->name, " to ", targetList->name);
#endif

    // remove from inactiveList

    if (element.previous > 0) elements[element.previous].next = element.next;
    if (element.next > 0) elements[element.next].previous = element.previous;

    // if this element is tail on inactive list then shift it back
    if (previousList->head == plod->index)
    {
#if PRINT_CONTAINER
        debug("   removing head from ", previousList->name);
#endif
        previousList->head = element.next;
    }

    if (previousList->tail == plod->index)
    {
#if PRINT_CONTAINER
        debug("   removing tail from ", previousList->name);
#endif
        previousList->tail = element.previous;
    }

    element.list = targetList;
    element.previous = targetList->tail;
    element.next = 0;

    // add to end of activeList tail
    if (targetList->head == 0)
    {
#if PRINT_CONTAINER
        debug("   setting ", targetList->name, ".head to", plod->index);
#endif
        targetList->head = plod->index;
    }

    if (targetList->tail > 0)
    {
#if PRINT_CONTAINER
        debug("   moving ", targetList->name, ".tail to ", plod->index);
#endif
        elements[targetList->tail].next = plod->index;
    }
    targetList->tail = plod->index;

    --(previousList->count);
    ++(targetList->count);
}

void PagedLODContainer::active(const PagedLOD* plod)
{
    debug("Moving to activeList", plod, ", ", plod->index);

    _move(plod, &activeList);

#if PRINT_CONTAINER
    debug_stream([&](auto& fout) { check(); print(fout); });
#endif
}

void PagedLODContainer::inactive(const PagedLOD* plod)
{
    debug("Moving to inactiveList", plod, ", ", plod->index);

    _move(plod, &inactiveList);

#if PRINT_CONTAINER
    debug_stream([&](std::ostream& fout) { check(); print(fout); });
#endif
}

void PagedLODContainer::remove(PagedLOD* plod)
{
    debug("Remove and make available to availableList", plod, ", ", plod->index);

    if (plod->index == 0)
    {
        warn("PagedLODContainer::remove() plod not assigned so ignore");
        check();
        return;
    }

    _move(plod, &availableList);

    // reset element and plod
    auto& element = elements[plod->index];
    plod->index = 0;
    element.plod = nullptr;

#if PRINT_CONTAINER
    check();
#endif
#if CHECK_CONTAINER
    info_stream([&](std::ostream& fout) { print(fout); });
#endif
}

bool PagedLODContainer::check(const List& list)
{
    if (list.head == 0)
    {
        // we have an empty list
        if (list.tail == 0)
        {
            if (list.count == 0) return true;
            warn("list ", list.name, " has a head==0 and tail==0 but length is ", list.count);
            return false;
        }

        warn("list ", list.name, " has a head==0, but tail is non zero");
        return false;
    }

    const auto& head_element = elements[list.head];
    if (head_element.previous != 0)
    {
        warn("list ", list.name, " has a head.previous that is non zero ", head_element.previous);
        return false;
    }

    const auto& tail_element = elements[list.tail];
    if (tail_element.next != 0)
    {
        warn("list ", list.name, " has a tail.next that is non zero ", tail_element.next);
        return false;
    }

    uint32_t count = 0;
    for (uint32_t i = list.head; i > 0 && count < elements.size();)
    {
        auto& element = elements[i];
        if (element.previous == 0)
        {
            if (i != list.head)
            {
                warn("list ", list.name, " non head element ", i, " has a previous==0");
                return false;
            }
        }
        else
        {
            auto& previous_element = elements[element.previous];
            if (previous_element.next != i)
            {
                warn("list ", list.name, " element = ", i, ", element.previous = ", element.previous, ", does not match to previous.next = ", previous_element.next);
                return false;
            }
        }

        if (element.next == 0)
        {
            if (i != list.tail)
            {
                warn("list ", list.name, " non tail element ", i, " has a next==0");
                return false;
            }
        }
        else
        {
            auto& next_element = elements[element.next];
            if (next_element.previous != i)
            {
                warn("list ", list.name, " element = ", i, ", element.next = ", element.next, ", does not match to next.previous = ", next_element.previous);
                return false;
            }
        }

        ++count;

        i = element.next;
    }

    if (count == list.count) return true;
    return false;
}

bool PagedLODContainer::check()
{
    bool result1 = check(availableList);
    bool result2 = check(activeList);
    bool result3 = check(inactiveList);
    return result1 && result2 && result3;
}
