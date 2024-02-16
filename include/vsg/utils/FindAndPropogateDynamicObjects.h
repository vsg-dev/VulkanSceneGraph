#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>

#include <stack>

namespace vsg
{

    /// Find all the objects that should be treated as dynamic (their values change.)
    class VSG_DECLSPEC FindDynamicObjects : public vsg::ConstVisitor
    {
    public:

        std::set<const Object*> dynamicObjects;

        inline void tag(const vsg::Object* object)
        {
            dynamicObjects.insert(object);
        }

        inline bool tagged(const vsg::Object* object)
        {
            return dynamicObjects.count(object) != 0;
        }

    protected:

        void apply(const vsg::Object& object) override;
        void apply(const vsg::Data& data) override;
        void apply(const vsg::AnimationGroup& ag) override;
        void apply(const vsg::Animation& animation) override;
        void apply(const vsg::AnimationSampler& sampler) override;
        void apply(const vsg::TransformSampler& sampler) override;
        void apply(const vsg::MorphSampler& sampler) override;
        void apply(const vsg::JointSampler& sampler) override;
        void apply(const vsg::BufferInfo& info) override;
        void apply(const vsg::Image& image) override;
        void apply(const vsg::ImageView& imageView) override;
        void apply(const vsg::ImageInfo& info) override;
        void apply(const vsg::DescriptorBuffer& db) override;
        void apply(const vsg::DescriptorImage& di) override;
        void apply(const vsg::BindIndexBuffer& bib) override;
        void apply(const vsg::BindVertexBuffers& bvb) override;
        void apply(const vsg::VertexDraw& vd) override;
        void apply(const vsg::VertexIndexDraw& vid) override;
        void apply(const vsg::Geometry& geom) override;
    };
    VSG_type_name(vsg::FindDynamicObjects);

    /// Propagate classification of objects as dynamic to all parents
    class VSG_DECLSPEC PropogateDynamicObjects : public vsg::ConstVisitor
    {
    public:

        PropogateDynamicObjects();

        std::set<const Object*> dynamicObjects;
        std::stack<bool> taggedStack;

        inline void tag(const vsg::Object* object)
        {
            dynamicObjects.insert(object);
        }

        inline bool tagged(const vsg::Object* object)
        {
            return dynamicObjects.count(object) != 0;
        }

    protected:

        struct TagIfChildIsDynamic
        {
            inline TagIfChildIsDynamic(PropogateDynamicObjects* in_rd, const vsg::Object* in_object) :
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

            PropogateDynamicObjects* rd = nullptr;
            const vsg::Object* object = nullptr;
        };

        void apply(const vsg::Object& object) override;
        void apply(const vsg::AnimationGroup& ag) override;
        void apply(const vsg::Animation& animation) override;
        void apply(const vsg::AnimationSampler& sampler) override;
        void apply(const vsg::TransformSampler& sampler) override;
        void apply(const vsg::MorphSampler& sampler) override;
        void apply(const vsg::JointSampler& sampler) override;
        void apply(const vsg::BufferInfo& info) override;
        void apply(const vsg::Image& image) override;
        void apply(const vsg::ImageView& imageView) override;
        void apply(const vsg::ImageInfo& info) override;
        void apply(const vsg::DescriptorBuffer& db) override;
        void apply(const vsg::DescriptorImage& di) override;
        void apply(const vsg::BindIndexBuffer& bib) override;
        void apply(const vsg::BindVertexBuffers& bvb) override;
        void apply(const vsg::VertexDraw& vd) override;
        void apply(const vsg::VertexIndexDraw& vid) override;
        void apply(const vsg::Geometry& geom) override;
    };
    VSG_type_name(vsg::PropogateDynamicObjects);

} // namespace vsg
