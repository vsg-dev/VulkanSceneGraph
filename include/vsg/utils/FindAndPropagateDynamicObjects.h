#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Inherit.h>

#include <stack>

namespace vsg
{

    /// Find all the objects that should be treated as dynamic (their values change.)
    class VSG_DECLSPEC FindDynamicObjects : public Inherit<ConstVisitor, FindDynamicObjects>
    {
    public:
        std::set<const Object*> dynamicObjects;

        inline void tag(const Object* object)
        {
            dynamicObjects.insert(object);
        }

        inline bool tagged(const Object* object)
        {
            return dynamicObjects.count(object) != 0;
        }

    protected:
        void apply(const Object& object) override;
        void apply(const Data& data) override;
        void apply(const AnimationGroup& ag) override;
        void apply(const Animation& animation) override;
        void apply(const AnimationSampler& sampler) override;
        void apply(const TransformSampler& sampler) override;
        void apply(const MorphSampler& sampler) override;
        void apply(const JointSampler& sampler) override;
        void apply(const BufferInfo& info) override;
        void apply(const Image& image) override;
        void apply(const ImageView& imageView) override;
        void apply(const ImageInfo& info) override;
        void apply(const DescriptorBuffer& db) override;
        void apply(const DescriptorImage& di) override;
        void apply(const BindIndexBuffer& bib) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const VertexDraw& vd) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const Geometry& geom) override;
    };
    VSG_type_name(FindDynamicObjects);

    /// Propagate classification of objects as dynamic to all parents
    class VSG_DECLSPEC PropagateDynamicObjects : public Inherit<ConstVisitor, PropagateDynamicObjects>
    {
    public:
        PropagateDynamicObjects();

        std::set<const Object*> dynamicObjects;
        std::stack<bool> taggedStack;

        inline void tag(const Object* object)
        {
            dynamicObjects.insert(object);
        }

        inline bool tagged(const Object* object)
        {
            return dynamicObjects.count(object) != 0;
        }

    protected:
        struct TagIfChildIsDynamic
        {
            inline TagIfChildIsDynamic(PropagateDynamicObjects* in_rd, const Object* in_object) :
                rd(in_rd),
                object(in_object)
            {
                if (rd->tagged(object)) rd->taggedStack.top() = true;
                rd->taggedStack.push(false);
            }

            inline ~TagIfChildIsDynamic()
            {
                if (rd->taggedStack.top())
                {
                    rd->tag(object);
                    rd->taggedStack.pop();
                    rd->taggedStack.top() = true;
                }
                else
                {
                    rd->taggedStack.pop();
                }
            }

            PropagateDynamicObjects* rd = nullptr;
            const Object* object = nullptr;
        };

        void apply(const Object& object) override;
        void apply(const AnimationGroup& ag) override;
        void apply(const Animation& animation) override;
        void apply(const AnimationSampler& sampler) override;
        void apply(const TransformSampler& sampler) override;
        void apply(const MorphSampler& sampler) override;
        void apply(const JointSampler& sampler) override;
        void apply(const BufferInfo& info) override;
        void apply(const Image& image) override;
        void apply(const ImageView& imageView) override;
        void apply(const ImageInfo& info) override;
        void apply(const DescriptorBuffer& db) override;
        void apply(const DescriptorImage& di) override;
        void apply(const BindIndexBuffer& bib) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const VertexDraw& vd) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const Geometry& geom) override;
    };
    VSG_type_name(PropagateDynamicObjects);

} // namespace vsg
