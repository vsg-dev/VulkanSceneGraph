/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/InstanceNode.h>
#include <vsg/vk/Context.h>

using namespace vsg;

InstanceNode::InstanceNode()
{
}

InstanceNode::InstanceNode(const InstanceNode& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    bound(rhs.bound),
    firstInstance(rhs.firstInstance),
    instanceCount(rhs.instanceCount),
    translations(copyop(rhs.translations)),
    rotations(copyop(rhs.rotations)),
    scales(copyop(rhs.scales)),
    colors(copyop(rhs.colors)),
    child(copyop(rhs.child))
{
}

InstanceNode::InstanceNode(const dsphere& in_bound, Node* in_child) :
    bound(in_bound),
    child(in_child)
{
}

InstanceNode::~InstanceNode()
{
}

int InstanceNode::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(bound, rhs.bound)) != 0) return result;
    if ((result = compare_value(firstInstance, rhs.firstInstance)) != 0) return result;
    if ((result = compare_value(instanceCount, rhs.instanceCount)) != 0) return result;
    if ((result = compare_pointer(translations, rhs.translations)) != 0) return result;
    if ((result = compare_pointer(rotations, rhs.rotations)) != 0) return result;
    if ((result = compare_pointer(scales, rhs.scales)) != 0) return result;
    if ((result = compare_pointer(colors, rhs.colors)) != 0) return result;
    return compare_pointer(child, rhs.child);
}

void InstanceNode::read(Input& input)
{
    Node::read(input);

    input.read("bound", bound);
    input.read("firstInstance", firstInstance);
    input.read("instanceCount", instanceCount);

    vsg::ref_ptr<Data> data;
    input.readObject("translations", data);
    if (data) translations = vsg::BufferInfo::create(data);
    else translations = {};

    input.readObject("rotations", data);
    if (data) rotations = vsg::BufferInfo::create(data);
    else rotations = {};

    input.readObject("scales", scales);
    if (data) scales = vsg::BufferInfo::create(data);
    else scales = {};

    input.readObject("colors", colors);
    if (data) colors = vsg::BufferInfo::create(data);
    else colors = {};

    input.read("child", child);
}

void InstanceNode::write(Output& output) const
{
    Node::write(output);

    output.write("bound", bound);
    output.write("firstInstance", firstInstance);
    output.write("instanceCount", instanceCount);

    if (translations) output.writeObject("translations", translations->data);
    else output.writeObject("translations", nullptr);

    if (rotations) output.writeObject("rotations", rotations->data);
    else output.writeObject("rotations", nullptr);

    if (scales) output.writeObject("scales", scales->data);
    else output.writeObject("scales", nullptr);

    if (colors) output.writeObject("colors", colors);
    else output.writeObject("colors", nullptr);

    output.write("child", child);
}

void InstanceNode::compile(Context& context)
{
    auto deviceID = context.deviceID;
    bool requiresCreateAndCopy = false;

    if (translations && translations->requiresCopy(deviceID)) requiresCreateAndCopy = true;
    if (rotations && rotations->requiresCopy(deviceID)) requiresCreateAndCopy = true;
    if (scales && scales->requiresCopy(deviceID)) requiresCreateAndCopy = true;
    if (colors && colors->requiresCopy(deviceID)) requiresCreateAndCopy = true;

    if (requiresCreateAndCopy)
    {
        BufferInfoList combinedBufferInfos;
        if (translations) combinedBufferInfos.push_back(translations);
        if (rotations) combinedBufferInfos.push_back(rotations);
        if (scales) combinedBufferInfos.push_back(scales);
        if (colors) combinedBufferInfos.push_back(colors);

        createBufferAndTransferData(context, combinedBufferInfos, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    }
}
