#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

#include <vsg/viewer/Camera.h>
#include <vsg/viewer/Window.h>

#include <set>

namespace vsg
{
    class VSG_DECLSPEC WindowResizeHandler : public Inherit<Visitor, WindowResizeHandler>
    {
    public:
        ref_ptr<Context> context;
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

        void scale_rect(VkRect2D& rect);

        /// return true if the object visited
        bool visit(const Object* object, uint32_t index = 0);

        void apply(BindGraphicsPipeline& bindPipeline) override;
        void apply(Object& object) override;
        void apply(StateGroup& sg) override;
        void apply(ClearAttachments& clearAttachments) override;
        void apply(View& view) override;
    };
    VSG_type_name(vsg::WindowResizeHandler);

} // namespace vsg
