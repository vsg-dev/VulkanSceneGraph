/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Camera
//
Camera::Camera() :
    projectionMatrix(Perspective::create()),
    viewMatrix(LookAt::create())
{
}

Camera::Camera(ref_ptr<ProjectionMatrix> in_projectionMatrix, ref_ptr<ViewMatrix> in_viewMatrix, ref_ptr<ViewportState> in_viewportState) :
    projectionMatrix(in_projectionMatrix),
    viewMatrix(in_viewMatrix),
    viewportState(in_viewportState)
{
}

void Camera::read(Input& input)
{
    Node::read(input);

    input.read("name", name);
    input.readObject("projectionMatrix", projectionMatrix);
    input.readObject("viewMatrix", viewMatrix);
    input.readObject("viewportState", viewportState);
}

void Camera::write(Output& output) const
{
    Node::write(output);

    output.write("name", name);
    output.writeObject("projectionMatrix", projectionMatrix);
    output.writeObject("viewMatrix", viewMatrix);
    output.writeObject("viewportState", viewportState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FindCameras
//
void FindCameras::apply(Object& object)
{
    _objectPath.push_back(&object);

    object.traverse(*this);

    _objectPath.pop_back();
}

void FindCameras::apply(Camera& camera)
{
    _objectPath.push_back(&camera);

    RefObjectPath convertedPath(_objectPath.begin(), _objectPath.end());

    cameras[convertedPath] = &camera;

    _objectPath.pop_back();
}
