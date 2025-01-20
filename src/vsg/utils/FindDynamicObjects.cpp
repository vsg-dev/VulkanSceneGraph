/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/JointSampler.h>
#include <vsg/animation/MorphSampler.h>
#include <vsg/animation/TransformSampler.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Draw.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/utils/FindDynamicObjects.h>
#include <vsg/vk/DescriptorPool.h>

using namespace vsg;

void FindDynamicObjects::apply(const Object& object)
{
    object.traverse(*this);
}

void FindDynamicObjects::apply(const Data& data)
{
    if (data.dynamic()) tag(&data);
}

void FindDynamicObjects::apply(const AnimationGroup& ag)
{
    tag(&ag);
    ag.traverse(*this);
}

void FindDynamicObjects::apply(const Animation& animation)
{
    tag(&animation);
    animation.traverse(*this);
}

void FindDynamicObjects::apply(const AnimationSampler& sampler)
{
    tag(&sampler);
}

void FindDynamicObjects::apply(const TransformSampler& sampler)
{
    tag(&sampler);
    if (sampler.object)
    {
        tag(sampler.object);
        sampler.object->traverse(*this);
    }
}

void FindDynamicObjects::apply(const MorphSampler& sampler)
{
    tag(&sampler);
    tag(sampler.object);
}

void FindDynamicObjects::apply(const JointSampler& sampler)
{
    tag(&sampler);
    tag(sampler.jointMatrices);
    tag(sampler.subgraph);
    sampler.subgraph->traverse(*this);
}

void FindDynamicObjects::apply(const BufferInfo& info)
{
    if (info.data) info.data->accept(*this);
}

void FindDynamicObjects::apply(const Image& image)
{
    if (image.data) image.data->accept(*this);
}

void FindDynamicObjects::apply(const ImageView& imageView)
{
    if (imageView.image) imageView.image->accept(*this);
}

void FindDynamicObjects::apply(const ImageInfo& info)
{
    if (info.sampler) info.sampler->accept(*this);
    if (info.imageView) info.imageView->accept(*this);
}

void FindDynamicObjects::apply(const DescriptorBuffer& db)
{
    for (auto info : db.bufferInfoList)
    {
        info->accept(*this);
    }
}

void FindDynamicObjects::apply(const DescriptorImage& di)
{
    for (auto info : di.imageInfoList)
    {
        info->accept(*this);
    }
}

void FindDynamicObjects::apply(const BindIndexBuffer& bib)
{
    if (bib.indices) bib.indices->accept(*this);
}

void FindDynamicObjects::apply(const BindVertexBuffers& bvb)
{
    for (auto info : bvb.arrays)
    {
        if (info) info->accept(*this);
    }
}

void FindDynamicObjects::apply(const VertexDraw& vd)
{
    for (auto info : vd.arrays)
    {
        if (info) info->accept(*this);
    }
}

void FindDynamicObjects::apply(const VertexIndexDraw& vid)
{
    if (vid.indices) vid.indices->accept(*this);
    for (auto info : vid.arrays)
    {
        if (info) info->accept(*this);
    }
}

void FindDynamicObjects::apply(const Geometry& geom)
{
    if (geom.indices) geom.indices->accept(*this);
    for (auto info : geom.arrays)
    {
        if (info) info->accept(*this);
    }
    for (auto command : geom.commands)
    {
        if (command) command->accept(*this);
    }
}
