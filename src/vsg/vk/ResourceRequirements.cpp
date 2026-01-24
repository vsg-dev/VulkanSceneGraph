/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/View.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/InstanceDraw.h>
#include <vsg/nodes/InstanceDrawIndexed.h>
#include <vsg/nodes/InstanceNode.h>
#include <vsg/nodes/Layer.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ResourceRequirements.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////
//
// ResourceRequirements
//
ResourceRequirements::ResourceRequirements()
{
    viewDetailsStack.push(ResourceRequirements::ViewDetails{});
}

ResourceRequirements::ResourceRequirements(ref_ptr<ResourceHints> hints) :
    vsg::ResourceRequirements()
{
    if (hints) apply(*hints);
}

uint32_t ResourceRequirements::computeNumDescriptorSets() const
{
    return externalNumDescriptorSets + static_cast<uint32_t>(descriptorSets.size());
}

DescriptorPoolSizes ResourceRequirements::computeDescriptorPoolSizes() const
{
    DescriptorPoolSizes poolSizes;
    for (auto& [type, count] : descriptorTypeMap)
    {
        poolSizes.push_back(VkDescriptorPoolSize{type, count});
    }
    return poolSizes;
}

void ResourceRequirements::apply(const ResourceHints& resourceHints)
{
    maxSlots.merge(resourceHints.maxSlots);

    if (!resourceHints.descriptorPoolSizes.empty() || resourceHints.numDescriptorSets > 0)
    {
        externalNumDescriptorSets += resourceHints.numDescriptorSets;

        for (auto& [type, count] : resourceHints.descriptorPoolSizes)
        {
            descriptorTypeMap[type] += count;
        }
    }

    minimumBufferSize = std::max(minimumBufferSize, resourceHints.minimumBufferSize);
    minimumDeviceMemorySize = std::max(minimumDeviceMemorySize, resourceHints.minimumDeviceMemorySize);
    minimumStagingBufferSize = std::max(minimumStagingBufferSize, resourceHints.minimumStagingBufferSize);

    numLightsRange = std::max(numLightsRange, resourceHints.numLightsRange);
    numShadowMapsRange = std::max(numShadowMapsRange, resourceHints.numShadowMapsRange);
    shadowMapSize = std::max(shadowMapSize, resourceHints.shadowMapSize);

    dataTransferHint = resourceHints.dataTransferHint;
    viewportStateHint = resourceHints.viewportStateHint;

    dynamicData.add(resourceHints.dynamicData);
    containsPagedLOD = containsPagedLOD | resourceHints.containsPagedLOD;
}

//////////////////////////////////////////////////////////////////////
//
// CollectResourceRequirements
//
ref_ptr<ResourceHints> CollectResourceRequirements::createResourceHints(uint32_t tileMultiplier) const
{
    auto resourceHints = vsg::ResourceHints::create();

    resourceHints->maxSlots = requirements.maxSlots;
    resourceHints->numDescriptorSets = static_cast<uint32_t>(requirements.computeNumDescriptorSets() * tileMultiplier);
    resourceHints->descriptorPoolSizes = requirements.computeDescriptorPoolSizes();

    for (auto& poolSize : resourceHints->descriptorPoolSizes)
    {
        poolSize.descriptorCount = poolSize.descriptorCount * tileMultiplier;
    }

    resourceHints->dynamicData = requirements.dynamicData;
    resourceHints->containsPagedLOD = requirements.containsPagedLOD;

    resourceHints->noTraverseBelowResourceHints = true;

    return resourceHints;
}

void CollectResourceRequirements::apply(const Object& object)
{
    object.traverse(*this);
}

bool CollectResourceRequirements::checkForResourceHints(const Object& object)
{
    auto resourceHints = object.getObject<ResourceHints>("ResourceHints");
    if (resourceHints)
    {
        apply(*resourceHints);
        return false; //resourceHints->noTraverseBelowResourceHints;
    }
    else
    {
        return false;
    }
}

void CollectResourceRequirements::apply(const ResourceHints& resourceHints)
{
    requirements.apply(resourceHints);
}

void CollectResourceRequirements::apply(const Node& node)
{
    if (checkForResourceHints(node))
    {
        return;
    }

    node.traverse(*this);
}

void CollectResourceRequirements::apply(const PagedLOD& plod)
{
    requirements.containsPagedLOD = true;

    if (checkForResourceHints(plod))
    {
        return;
    }

    plod.traverse(*this);
}

void CollectResourceRequirements::apply(const StateCommand& stateCommand)
{
    if (stateCommand.slot > requirements.maxSlots.state) requirements.maxSlots.state = stateCommand.slot;
    stateCommand.traverse(*this);
}

void CollectResourceRequirements::apply(const DescriptorSet& descriptorSet)
{
    if (requirements.descriptorSets.count(&descriptorSet) == 0)
    {
        requirements.descriptorSets.insert(&descriptorSet);

        descriptorSet.traverse(*this);
    }
}

bool CollectResourceRequirements::registerDescriptor(const Descriptor& descriptor)
{
    requirements.descriptorTypeMap[descriptor.descriptorType] += descriptor.getNumDescriptors();

    if (requirements.descriptors.count(&descriptor) == 0)
    {
        requirements.descriptors.insert(&descriptor);
        return true;
    }
    else
    {
        return false;
    }
}

void CollectResourceRequirements::apply(const Descriptor& descriptor)
{
    registerDescriptor(descriptor);
}

void CollectResourceRequirements::apply(const DescriptorBuffer& descriptorBuffer)
{
    if (registerDescriptor(descriptorBuffer))
    {
        BufferProperties properties;

        switch (descriptorBuffer.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            properties.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            properties.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        default:
            break;
        }

        properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        properties.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        //info("CollectResourceRequirements::apply(const DescriptorBuffer& descriptorBuffer) ", &descriptorBuffer);
        for (const auto& bufferInfo : descriptorBuffer.bufferInfoList) apply(bufferInfo, properties);
    }
}

void CollectResourceRequirements::apply(const DescriptorImage& descriptorImage)
{
    if (registerDescriptor(descriptorImage))
    {
        //info("CollectResourceRequirements::apply(const DescriptorImage& descriptorImage) ", &descriptorImage);
        for (const auto& imageInfo : descriptorImage.imageInfoList) apply(imageInfo);
    }
}

void CollectResourceRequirements::apply(const Light& light)
{
    requirements.viewDetailsStack.top().lights.insert(&light);
}

void CollectResourceRequirements::apply(const RenderGraph& rg)
{
    if (rg.viewportState) requirements.maxSlots.view = std::max(requirements.maxSlots.view, rg.viewportState->slot);
    rg.traverse(*this);
}

void CollectResourceRequirements::apply(const View& view)
{
    if (view.camera && view.camera->viewportState)
    {
        requirements.maxSlots.view = std::max(requirements.maxSlots.view, view.camera->viewportState->slot);
    }

    if (auto itr = requirements.views.find(&view); itr != requirements.views.end())
    {
        requirements.viewDetailsStack.push(itr->second);
    }
    else
    {
        requirements.viewDetailsStack.push(ResourceRequirements::ViewDetails{});
    }

    view.traverse(*this);

    auto& viewDetails = requirements.viewDetailsStack.top();

    for (auto& bin : view.bins)
    {
        viewDetails.bins.insert(bin);
    }

    requirements.views[&view] = viewDetails;

    if (view.viewDependentState)
    {
        view.viewDependentState->init(requirements);

        view.viewDependentState->accept(*this);
    }

    requirements.viewDetailsStack.pop();
}

void CollectResourceRequirements::apply(const DepthSorted& depthSorted)
{
    requirements.viewDetailsStack.top().indices.insert(depthSorted.binNumber);

    depthSorted.traverse(*this);
}

void CollectResourceRequirements::apply(const Layer& layer)
{
    requirements.viewDetailsStack.top().indices.insert(layer.binNumber);

    layer.traverse(*this);
}

void CollectResourceRequirements::apply(const Bin& bin)
{
    requirements.viewDetailsStack.top().bins.insert(&bin);
}

void CollectResourceRequirements::apply(const Geometry& geometry)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (geometry.indices) properties.usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    for (const auto& bufferInfo : geometry.arrays) apply(bufferInfo, properties);

    if (geometry.indices) apply(geometry.indices, properties);
}

void CollectResourceRequirements::apply(const VertexDraw& vd)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    for (const auto& bufferInfo : vd.arrays) apply(bufferInfo, properties);
}

void CollectResourceRequirements::apply(const VertexIndexDraw& vid)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    for (const auto& bufferInfo : vid.arrays) apply(bufferInfo, properties);
    apply(vid.indices, properties);
}

void CollectResourceRequirements::apply(const InstanceNode& in)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (in.translations) apply(in.translations, properties);
    if (in.rotations) apply(in.rotations, properties);
    if (in.scales) apply(in.scales, properties);
    if (in.colors) apply(in.colors, properties);

    in.traverse(*this);
}

void CollectResourceRequirements::apply(const InstanceDraw& id)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    for (const auto& bufferInfo : id.arrays) apply(bufferInfo, properties);
}

void CollectResourceRequirements::apply(const InstanceDrawIndexed& idi)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    for (const auto& bufferInfo : idi.arrays) apply(bufferInfo, properties);
    apply(idi.indices, properties);
}

void CollectResourceRequirements::apply(const BindVertexBuffers& bvb)
{
    BufferProperties properties{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    for (const auto& bufferInfo : bvb.arrays) apply(bufferInfo, properties);
}

void CollectResourceRequirements::apply(const BindIndexBuffer& bib)
{
    BufferProperties properties{VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE};
    if (requirements.dataTransferHint == COMPILE_TRAVERSAL_USE_TRANSFER_TASK) properties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    apply(bib.indices, properties);
}

void CollectResourceRequirements::apply(ref_ptr<BufferInfo> bufferInfo, BufferProperties bufferProperties)
{
    if (bufferInfo)
    {
        if (bufferInfo->data && bufferInfo->data->dynamic()) bufferProperties.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        auto& bufferInfos = requirements.bufferInfos[bufferProperties];
        if (bufferInfos.count(bufferInfo) == 0)
        {
            bufferInfos.insert(bufferInfo);

            if (bufferInfo->data)
            {
                if (bufferInfo->data->dynamic())
                {
                    requirements.dynamicData.bufferInfos.push_back(bufferInfo);
                }
            }
        }
    }
}

void CollectResourceRequirements::apply(ref_ptr<ImageInfo> imageInfo)
{
    if (imageInfo && requirements.imageInfos.count(imageInfo) == 0)
    {
        if (imageInfo->imageView && imageInfo->imageView->image)
        {
            requirements.imageInfos.insert(imageInfo);

            // check for dynamic data
            auto& data = imageInfo->imageView->image->data;
            if (data)
            {
                if (data->dynamic())
                {
                    requirements.dynamicData.imageInfos.push_back(imageInfo);
                }
            }
        }
        else
        {
            vsg::debug("CollectResourceRequirements::apply() problem ImageInfo ", imageInfo, " { ", imageInfo->imageView, "}");
        }
    }
}
