/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

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
    indexCount(rhs.indexCount),
    firstIndex(rhs.firstIndex),
    vertexOffset(rhs.vertexOffset),
    firstBinding(rhs.firstBinding),
    arrays(copyop(rhs.arrays)),
    indices(copyop(rhs.indices))
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
    if ((result = compare_value(indexCount, rhs.indexCount)) != 0) return result;
    if ((result = compare_value(firstIndex, rhs.firstIndex)) != 0) return result;
    if ((result = compare_value(vertexOffset, rhs.vertexOffset)) != 0) return result;
    if ((result = compare_value(firstBinding, rhs.firstBinding)) != 0) return result;
    if ((result = compare_pointer_container(arrays, rhs.arrays)) != 0) return result;
    return compare_pointer(indices, rhs.indices);
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

void InstanceDraw::assignIndices(ref_ptr<vsg::Data> indexData)
{
    if (indexData)
    {
        indices = BufferInfo::create(indexData);
        indexType = computeIndexType(indices->data);
    }
    else
    {
        indices = {};
    }
}

void InstanceDraw::read(Input& input)
{
    _vulkanData.clear();

    Command::read(input);

    input.read("firstBinding", firstBinding);

    DataList dataList;
    dataList.resize(input.readValue<uint32_t>("NumArrays"));
    for (auto& array : dataList)
    {
        input.readObject("Array", array);
    }
    assignArrays(dataList);

    ref_ptr<vsg::Data> indices_data;
    input.readObject("Indices", indices_data);

    assignIndices(indices_data);

    // vkCmdDrawIndexed settings
    input.read("indexCount", indexCount);
    input.read("firstIndex", firstIndex);
    input.read("vertexOffset", vertexOffset);
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

    if (indices)
        output.writeObject("Indices", indices->data.get());
    else
        output.writeObject("Indices", nullptr);

    // vkCmdDrawIndexed settings
    output.write("indexCount", indexCount);
    output.write("firstIndex", firstIndex);
    output.write("vertexOffset", vertexOffset);
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
    if (indices && indices->requiresCopy(deviceID))
        requiresCreateAndCopy = true;
    else
    {
        for (auto& array : arrays)
        {
            if (array->requiresCopy(deviceID))
            {
                requiresCreateAndCopy = true;
                break;
            }
        }
    }

    if (requiresCreateAndCopy)
    {
        BufferInfoList combinedBufferInfos(arrays);
        if (indices) combinedBufferInfos.push_back(indices);
        createBufferAndTransferData(context, combinedBufferInfos, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);

        // info("InstanceDraw::compile() create and copy ", this);
    }
    else
    {
        // info("InstanceDraw::compile() no need to create and copy ", this);
    }

    assignVulkanArrayData(deviceID, arrays, _vulkanData[deviceID]);
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

    auto assignBufferInfo = [&](const ref_ptr<BufferInfo>& bufferInfo) -> void
    {
        vkBuffers.push_back(bufferInfo->buffer->vk(deviceID));
        offsets.push_back(bufferInfo->offset);
    };

    for(auto& bi : arrays)
    {
        assignBufferInfo(bi);
    }

    if (instanceNode->colors) assignBufferInfo(instanceNode->colors);
    if (instanceNode->translations) assignBufferInfo(instanceNode->translations);
    if (instanceNode->rotations) assignBufferInfo(instanceNode->rotations);
    if (instanceNode->scales) assignBufferInfo(instanceNode->scales);

    // TODO: will need to get the values to apply by combing the inherited InstanceNode values with local arrays
    vkCmdBindVertexBuffers(cmdBuffer, firstBinding, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());

    if (indices)
    {
        // vsg::info("InstanceDraw::record(CommandBuffer& commandBuffer) vkCmdDrawIndexed vkBuffers.size() = ", vkBuffers.size(), ", indexCount = ", indexCount, ", instanceNode->instanceCount = ", instanceNode->instanceCount);

        vkCmdBindIndexBuffer(cmdBuffer, indices->buffer->vk(deviceID), indices->offset, indexType);
        vkCmdDrawIndexed(cmdBuffer, indexCount, instanceNode->instanceCount, firstIndex, vertexOffset, instanceNode->firstInstance);
    }
    else
    {
        // vsg::info("InstanceDraw::record(CommandBuffer& commandBuffer) vkCmdDraw vkBuffers.size() = ", vkBuffers.size(), ", indexCount = ", indexCount, ", instanceNode->instanceCount = ", instanceNode->instanceCount);
        vkCmdDraw(cmdBuffer, indexCount, instanceNode->instanceCount, firstIndex, instanceNode->firstInstance);
    }

}
