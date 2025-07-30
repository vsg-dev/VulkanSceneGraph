/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/InstanceDraw.h>
#include <vsg/nodes/InstanceNode.h>
#include <vsg/vk/Context.h>

using namespace vsg;

InstanceDraw::InstanceDraw()
{
}

InstanceDraw::InstanceDraw(const InstanceDraw& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    vertexCount(rhs.vertexCount),
    firstVertex(rhs.firstVertex),
    firstBinding(rhs.firstBinding),
    arrays(copyop(rhs.arrays))
{
}

InstanceDraw::~InstanceDraw()
{
}

int InstanceDraw::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(vertexCount, rhs.vertexCount)) != 0) return result;
    if ((result = compare_value(firstVertex, rhs.firstVertex)) != 0) return result;
    if ((result = compare_value(firstBinding, rhs.firstBinding)) != 0) return result;
    return compare_pointer_container(arrays, rhs.arrays);
}

void InstanceDraw::assignArrays(const DataList& arrayData)
{
    arrays.clear();
    arrays.reserve(arrayData.size());
    for (auto& data : arrayData)
    {
        arrays.push_back(BufferInfo::create(data));
    }
}

void InstanceDraw::read(Input& input)
{
    Command::read(input);

    input.read("firstBinding", firstBinding);

    DataList dataList;
    dataList.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : dataList)
    {
        input.readObject("Array", array);
    }
    assignArrays(dataList);

    // vkCmdDraw settings
    input.read("vertexCount", vertexCount);
    input.read("firstVertex", firstVertex);
}

void InstanceDraw::write(Output& output) const
{
    Command::write(output);

    output.write("firstBinding", firstBinding);
    output.writeValue<uint32_t>("NumArrays", arrays.size());
    for (const auto& array : arrays)
    {
        if (array)
            output.writeObject("Array", array->data.get());
        else
            output.writeObject("Array", nullptr);
    }

    // vkCmdDrawIndexed settings
    output.write("vertexCount", vertexCount);
    output.write("firstVertex", firstVertex);
}

void InstanceDraw::compile(Context& context)
{
    if (arrays.empty())
    {
        // InstanceDraw does not contain required arrays and indices
        return;
    }

    auto deviceID = context.deviceID;

    bool requiresCreateAndCopy = false;
    for (auto& array : arrays)
    {
        if (array->requiresCopy(deviceID))
        {
            requiresCreateAndCopy = true;
            break;
        }
    }

    if (requiresCreateAndCopy)
    {
        createBufferAndTransferData(context, arrays, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

        // info("InstanceDraw::compile() create and copy ", this);
    }
    else
    {
        // info("InstanceDraw::compile() no need to create and copy ", this);
    }
}

void InstanceDraw::record(CommandBuffer& commandBuffer) const
{
    auto instanceNode = commandBuffer.instanceNode;
    if (!instanceNode)
    {
        vsg::info("InstanceDraw::record() required vsg::InstanceNode not provided.");
        return;
    }

    auto deviceID = commandBuffer.deviceID;
    VkCommandBuffer cmdBuffer{commandBuffer};

    std::vector<VkBuffer> vkBuffers;
    std::vector<VkDeviceSize> offsets;

    vkBuffers.reserve(8);
    offsets.reserve(8);

    auto assignBufferInfo = [&](const ref_ptr<BufferInfo>& bufferInfo) -> void {
        vkBuffers.push_back(bufferInfo->buffer->vk(deviceID));
        offsets.push_back(bufferInfo->offset);
    };

    for (auto& bi : arrays)
    {
        assignBufferInfo(bi);
    }

    if (instanceNode->colors) assignBufferInfo(instanceNode->colors);
    if (instanceNode->translations) assignBufferInfo(instanceNode->translations);
    if (instanceNode->rotations) assignBufferInfo(instanceNode->rotations);
    if (instanceNode->scales) assignBufferInfo(instanceNode->scales);

    // TODO: will need to get the values to apply by combing the inherited InstanceNode values with local arrays
    vkCmdBindVertexBuffers(cmdBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());

    // vsg::info("InstanceDraw::record(CommandBuffer& commandBuffer) vkCmdDraw vkBuffers.size() = ", vkBuffers.size(), ", vertexCount = ", vertexCount, ", instanceNode->instanceCount = ", instanceNode->instanceCount);
    vkCmdDraw(cmdBuffer, vertexCount, instanceNode->instanceCount, firstVertex, instanceNode->firstInstance);
}
