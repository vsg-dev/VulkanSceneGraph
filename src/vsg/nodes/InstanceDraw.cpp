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
    if (arrays.empty() || !indices)
    {
        // InstanceDraw does not contain required arrays and indices
        return;
    }

    auto deviceID = context.deviceID;

    bool requiresCreateAndCopy = false;
    if (indices->requiresCopy(deviceID))
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
        combinedBufferInfos.push_back(indices);
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
    vsg::info("InstanceDraw::record() commandBuffer.instanceNode = ", commandBuffer.instanceNode);

    return;

    auto& vkd = _vulkanData[commandBuffer.deviceID];

    VkCommandBuffer cmdBuffer{commandBuffer};

    // TODO: will need to get the values to apply by combing the inherited InstanceNode values with local arrays
    vkCmdBindVertexBuffers(cmdBuffer, firstBinding, static_cast<uint32_t>(vkd.vkBuffers.size()), vkd.vkBuffers.data(), vkd.offsets.data());

    vkCmdBindIndexBuffer(cmdBuffer, indices->buffer->vk(commandBuffer.deviceID), indices->offset, indexType);

    // TODO: hardwire for now, will need to get details from the inherited InstanceNode
    uint32_t instanceCount = 1;
    uint32_t firstInstance = 0;

    vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
