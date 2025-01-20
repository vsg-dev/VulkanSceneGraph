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
#include <vsg/utils/PropagateDynamicObjects.h>
#include <vsg/vk/DescriptorPool.h>

using namespace vsg;

PropagateDynamicObjects::PropagateDynamicObjects()
{
    taggedStack.push(false);
}

void PropagateDynamicObjects::apply(const Object& object)
{
    TagIfChildIsDynamic t(this, &object);
    object.traverse(*this);
}

void PropagateDynamicObjects::apply(const AnimationGroup& ag)
{
    TagIfChildIsDynamic t(this, &ag);
    ag.traverse(*this);
}

void PropagateDynamicObjects::apply(const Animation& animation)
{
    TagIfChildIsDynamic t(this, &animation);
    animation.traverse(*this);
}

void PropagateDynamicObjects::apply(const AnimationSampler& sampler)
{
    TagIfChildIsDynamic t(this, &sampler);
}

void PropagateDynamicObjects::apply(const TransformSampler& sampler)
{
    TagIfChildIsDynamic t(this, &sampler);
    if (sampler.object) sampler.object->accept(*this);
}

void PropagateDynamicObjects::apply(const MorphSampler& sampler)
{
    TagIfChildIsDynamic t(this, &sampler);
}

void PropagateDynamicObjects::apply(const JointSampler& sampler)
{
    TagIfChildIsDynamic t(this, &sampler);
    if (sampler.subgraph) sampler.subgraph->accept(*this);
}

void PropagateDynamicObjects::apply(const BufferInfo& info)
{
    TagIfChildIsDynamic t(this, &info);
    if (info.data) info.data->accept(*this);
}

void PropagateDynamicObjects::apply(const Image& image)
{
    TagIfChildIsDynamic t(this, &image);
    if (image.data) image.data->accept(*this);
}

void PropagateDynamicObjects::apply(const ImageView& imageView)
{
    TagIfChildIsDynamic t(this, &imageView);
    if (imageView.image) imageView.image->accept(*this);
}

void PropagateDynamicObjects::apply(const ImageInfo& info)
{
    TagIfChildIsDynamic t(this, &info);
    if (info.sampler) info.sampler->accept(*this);
    if (info.imageView) info.imageView->accept(*this);
}

void PropagateDynamicObjects::apply(const DescriptorBuffer& db)
{
    TagIfChildIsDynamic t(this, &db);
    for (auto info : db.bufferInfoList)
    {
        info->accept(*this);
    }
}

void PropagateDynamicObjects::apply(const DescriptorImage& di)
{
    TagIfChildIsDynamic t(this, &di);
    for (auto info : di.imageInfoList)
    {
        info->accept(*this);
    }
}

void PropagateDynamicObjects::apply(const BindIndexBuffer& bib)
{
    TagIfChildIsDynamic t(this, &bib);
    if (bib.indices) bib.indices->accept(*this);
}

void PropagateDynamicObjects::apply(const BindVertexBuffers& bvb)
{
    TagIfChildIsDynamic t(this, &bvb);
    for (auto info : bvb.arrays)
    {
        if (info) info->accept(*this);
    }
}

void PropagateDynamicObjects::apply(const VertexDraw& vd)
{
    TagIfChildIsDynamic t(this, &vd);
    for (auto info : vd.arrays)
    {
        if (info) info->accept(*this);
    }
}

void PropagateDynamicObjects::apply(const VertexIndexDraw& vid)
{
    TagIfChildIsDynamic t(this, &vid);
    if (vid.indices) vid.indices->accept(*this);
    for (auto info : vid.arrays)
    {
        if (info) info->accept(*this);
    }
}

void PropagateDynamicObjects::apply(const Geometry& geom)
{
    TagIfChildIsDynamic t(this, &geom);
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
