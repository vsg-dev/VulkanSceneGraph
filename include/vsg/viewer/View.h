#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Node.h>
#include <vsg/viewer/Camera.h>

namespace vsg
{
    class View : public Inherit<Object, View>
    {
    public:
        View();

        /// set the master Camear of the View
        void setCamera(ref_ptr<Camera> camera);
        Camera* getCamera() { return _camera; }
        const Camera* getCamera() const { return _camera; }

        void setScene(ref_ptr<Node> scene) { _scene = scene; }
        Node* getScene() { return _scene; }
        Node* getScene() const { return _scene; }

    protected:
        ref_ptr<Camera> _camera;
        ref_ptr<Node> _scene;
    };
} // namespace vsg
