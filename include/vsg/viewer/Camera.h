#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/GraphicsPipeline.h>

#include <vsg/viewer/ProjectionMatrix.h>
#include <vsg/viewer/ViewMatrix.h>

namespace vsg
{
    class Camera : public Inherit<Object, Camera>
    {
    public:
        Camera();

        Camera(ref_ptr<ProjectionMatrix> projectionMatrix, ref_ptr<ViewMatrix> viewMatrix, ref_ptr<ViewportState> viewportState);

        void setProjectionMatrix(ref_ptr<ProjectionMatrix> projectionMatrix) { _projectionMatrix = projectionMatrix; }
        ProjectionMatrix* getProjectionMatrix() const { return _projectionMatrix; }

        void setViewMatrix(ref_ptr<ViewMatrix> viewMatrix) { _viewMatrix = viewMatrix; }
        ViewMatrix* getViewMatrix() const { return _viewMatrix; }

        void setViewportState(ref_ptr<ViewportState> viewportState) { _viewportState = viewportState; }
        ViewportState* getViewportState() const { return _viewportState; }

        VkRect2D getRenderArea() const { return _viewportState ? _viewportState->getScissor() : VkRect2D{}; }

    protected:
        ref_ptr<ProjectionMatrix> _projectionMatrix;
        ref_ptr<ViewMatrix> _viewMatrix;
        ref_ptr<ViewportState> _viewportState;
    };
} // namespace vsg
