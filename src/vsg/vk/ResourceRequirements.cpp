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
#include <vsg/nodes/Layer.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/MultisampleState.h>
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
    if (resourceHints.maxSlot > maxSlot) maxSlot = resourceHints.maxSlot;

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
}

//////////////////////////////////////////////////////////////////////
//
// CollectResourceRequirements
//
ref_ptr<ResourceHints> CollectResourceRequirements::createResourceHints(uint32_t tileMultiplier) const
{
    auto resourceHints = vsg::ResourceHints::create();

    resourceHints->maxSlot = requirements.maxSlot;
    resourceHints->numDescriptorSets = static_cast<uint32_t>(requirements.computeNumDescriptorSets() * tileMultiplier);
    resourceHints->descriptorPoolSizes = requirements.computeDescriptorPoolSizes();

    for (auto& poolSize : resourceHints->descriptorPoolSizes)
    {
        poolSize.descriptorCount = poolSize.descriptorCount * tileMultiplier;
    }

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
        return true;
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
    bool hasResourceHints = checkForResourceHints(node);
    if (hasResourceHints) ++_numResourceHintsAbove;

    node.traverse(*this);

    if (hasResourceHints) --_numResourceHintsAbove;
}

void CollectResourceRequirements::apply(const PagedLOD& plod)
{
    bool hasResourceHints = checkForResourceHints(plod);
    if (hasResourceHints) ++_numResourceHintsAbove;

    requirements.containsPagedLOD = true;
    plod.traverse(*this);

    if (hasResourceHints) --_numResourceHintsAbove;
}

void CollectResourceRequirements::apply(const StateCommand& stateCommand)
{
    if (stateCommand.slot > requirements.maxSlot) requirements.maxSlot = stateCommand.slot;
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
        //info("CollectResourceRequirements::apply(const DescriptorBuffer& descriptorBuffer) ", &descriptorBuffer);
        for (const auto& bufferInfo : descriptorBuffer.bufferInfoList) apply(bufferInfo);
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
    if (rg.viewportState) rg.viewportState->accept(*this);
    rg.traverse(*this);
}

void CollectResourceRequirements::apply(const View& view)
{
    if (view.camera && view.camera->viewportState) view.camera->viewportState->accept(*this);

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
        if (requirements.maxSlot < 2) requirements.maxSlot = 2;

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
    for (const auto& bufferInfo : geometry.arrays) apply(bufferInfo);
    apply(geometry.indices);
}

void CollectResourceRequirements::apply(const VertexDraw& vd)
{
    for (const auto& bufferInfo : vd.arrays) apply(bufferInfo);
}

void CollectResourceRequirements::apply(const VertexIndexDraw& vid)
{
    for (const auto& bufferInfo : vid.arrays) apply(bufferInfo);
    apply(vid.indices);
}

void CollectResourceRequirements::apply(const BindVertexBuffers& bvb)
{
    for (const auto& bufferInfo : bvb.arrays) apply(bufferInfo);
}

void CollectResourceRequirements::apply(const BindIndexBuffer& bib)
{
    apply(bib.indices);
}

void CollectResourceRequirements::apply(ref_ptr<BufferInfo> bufferInfo)
{
    if (bufferInfo && bufferInfo->data && bufferInfo->data->dynamic())
    {
        requirements.dynamicData.bufferInfos.push_back(bufferInfo);
    }
}

void CollectResourceRequirements::apply(ref_ptr<ImageInfo> imageInfo)
{
    if (imageInfo && imageInfo->imageView && imageInfo->imageView->image)
    {
        // check for dynamic data
        auto& data = imageInfo->imageView->image->data;
        if (data && data->dynamic())
        {
            requirements.dynamicData.imageInfos.push_back(imageInfo);
        }
    }
}
