/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>

using namespace vsg;

#define USE_MUTEX 0

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(ref_ptr<DescriptorSetLayout> in_descriptorSetLayout, const Descriptors& in_descriptors) :
    setLayout(in_descriptorSetLayout),
    descriptors(in_descriptors)
{
}

DescriptorSet::~DescriptorSet()
{
    release();
}

int DescriptorSet::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer(setLayout, rhs.setLayout))) return result;
    return compare_pointer_container(descriptors, rhs.descriptors);
}

void DescriptorSet::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("setLayout", setLayout);
        input.readObjects("descriptors", descriptors);
    }
    else
    {
        input.read("DescriptorSetLayout", setLayout);

        descriptors.resize(input.readValue<uint32_t>("NumDescriptors"));
        for (auto& descriptor : descriptors)
        {
            input.read("Descriptor", descriptor);
        }
    }
}

void DescriptorSet::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("setLayout", setLayout);
        output.writeObjects("descriptors", descriptors);
    }
    else
    {
        output.write("DescriptorSetLayout", setLayout);

        output.writeValue<uint32_t>("NumDescriptors", descriptors.size());
        for (auto& descriptor : descriptors)
        {
            output.write("Descriptor", descriptor);
        }
    }
}

void DescriptorSet::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        // make sure all the contributing objects are compiled
        if (setLayout) setLayout->compile(context);
        for (auto& descriptor : descriptors) descriptor->compile(context);

#if 1
        _implementation[context.deviceID] = context.allocateDescriptorSet(setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
#else

#    if USE_MUTEX
        std::scoped_lock<std::mutex> lock(context.descriptorPool->getMutex());
#    endif
        _implementation[context.deviceID] = DescriptorSet::Implementation::create(context.descriptorPool, setLayout);
        _implementation[context.deviceID]->assign(context, descriptors);
#endif
    }
}

void DescriptorSet::release(uint32_t deviceID)
{
#if 1
    recyle(_implementation[deviceID]);
#else
    _implementation[deviceID] = {};
#endif
}
void DescriptorSet::release()
{
#if 1
    for (auto& dsi : _implementation) recyle(dsi);
#endif
    _implementation.clear();
}

VkDescriptorSet DescriptorSet::vk(uint32_t deviceID) const
{
    return _implementation[deviceID]->_descriptorSet;
}
