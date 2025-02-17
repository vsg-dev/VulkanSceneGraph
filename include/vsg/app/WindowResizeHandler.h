#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/app/Window.h>
#include <vsg/nodes/Group.h>

#include <set>

namespace vsg
{

    /// Utility class for updating a scene graph when a View's camera ViewportState has been updated so that associated GraphicsPipelines in the
    /// scene graph can be recompiled and correctly reflect the new ViewportState.
    /// As viewport size and scissor is dynamic by default, this is only necessary when opting out of that or when the viewport count has changed.
    class VSG_DECLSPEC UpdateGraphicsPipelines : public Inherit<Visitor, UpdateGraphicsPipelines>
    {
    public:
        UpdateGraphicsPipelines();

        ref_ptr<Context> context;
        std::set<std::pair<const Object*, uint32_t>> visited;

        bool visit(const Object* object, uint32_t index);

        void apply(Object& object) override;
        void apply(BindGraphicsPipeline& bindPipeline) override;
        void apply(View& view) override;
    };
    VSG_type_name(UpdateGraphicsPipelines);

    /// WindowResizeHandler class for updating viewport/scissor and attachments to fit with new window dimensions.
    class VSG_DECLSPEC WindowResizeHandler : public Inherit<Visitor, WindowResizeHandler>
    {
    public:
        VkRect2D renderArea;
        VkExtent2D previous_extent;
        VkExtent2D new_extent;
        std::set<std::pair<const Object*, uint32_t>> visited;

        WindowResizeHandler();

        template<typename T, typename R>
        T scale_parameter(T original, R extentOriginal, R extentNew)
        {
            if (original == static_cast<T>(extentOriginal)) return static_cast<T>(extentNew);
            return static_cast<T>(static_cast<float>(original) * static_cast<float>(extentNew) / static_cast<float>(extentOriginal) + 0.5f);
        }

        virtual void scale_rect(VkRect2D& rect);
        virtual void scale_viewport(VkViewport& viewport);

        /// return true if the object was visited
        bool visit(const Object* object, uint32_t index = 0);

        void apply(Object& object) override;
        void apply(ClearAttachments& clearAttachments) override;
        void apply(View& view) override;
    };
    VSG_type_name(WindowResizeHandler);

} // namespace vsg
