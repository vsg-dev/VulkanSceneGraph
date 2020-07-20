/* <editor-fold desc="MIT License">

Copyright(c) 2019 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Trackball.h>
#include <vsg/io/Options.h>

#include <iostream>

using namespace vsg;

Trackball::Trackball(ref_ptr<Camera> camera) :
    _camera(camera)
{
    _lookAt = dynamic_cast<LookAt*>(_camera->getViewMatrix());

    if (!_lookAt)
    {
        // TODO: need to work out how to map the original ViewMatrix to a LookAt and back, for now just fallback to assigning our own LookAt
        _lookAt = new LookAt;
    }

    _homeLookAt = new LookAt(_lookAt->eye, _lookAt->center, _lookAt->up);
}

/// compute non dimensional window coordinate (-1,1) from event coords
dvec2 Trackball::ndc(PointerEvent& event)
{
    dvec2 v = dvec2((static_cast<double>(event.x) / window_width) * 2.0 - 1.0, (static_cast<double>(event.y) / window_height) * 2.0 - 1.0);
    //std::cout<<"ndc = "<<v<<std::endl;
    return v;
}

/// compute trackball coordinate from event coords
dvec3 Trackball::tbc(PointerEvent& event)
{
    dvec2 v = ndc(event);

    double l = length(v);
    if (l < 1.0f)
    {
        double h = 0.5 + cos(l * PI) * 0.5;
        return dvec3(v.x, -v.y, h);
    }
    else
    {
        return dvec3(v.x, -v.y, 0.0);
    }
}

void Trackball::apply(KeyPressEvent& keyPress)
{
    if (keyPress.keyBase == _homeKey)
    {
        LookAt* lookAt = dynamic_cast<LookAt*>(_camera->getViewMatrix());
        if (lookAt && _homeLookAt)
        {
            lookAt->eye = _homeLookAt->eye;
            lookAt->center = _homeLookAt->center;
            lookAt->up = _homeLookAt->up;
        }
    }
}

void Trackball::apply(ConfigureWindowEvent& configureWindow)
{
    window_width = static_cast<double>(configureWindow.width);
    window_height = static_cast<double>(configureWindow.height);
    // std::cout<<"Trackball::apply(ConfigureWindowEvent& "<<configureWindow.x<<", "<<configureWindow.y<<", "<<window_width<<", "<<window_height<<")"<<std::endl;
}

void Trackball::apply(ButtonPressEvent& buttonPress)
{
    prev_ndc = ndc(buttonPress);
    prev_tbc = tbc(buttonPress);
}

void Trackball::apply(ButtonReleaseEvent& buttonRelease)
{
    prev_ndc = ndc(buttonRelease);
    prev_tbc = tbc(buttonRelease);
}

void Trackball::apply(MoveEvent& moveEvent)
{
    dvec2 new_ndc = ndc(moveEvent);
    dvec3 new_tbc = tbc(moveEvent);

    if (moveEvent.mask & BUTTON_MASK_1)
    {
        dvec3 xp = cross(normalize(new_tbc), normalize(prev_tbc));
        double xp_len = length(xp);
        if (xp_len > 0.0)
        {
            dvec3 axis = xp / xp_len;
            double angle = asin(xp_len);
            rotate(angle, axis);
        }
    }
    else if (moveEvent.mask & BUTTON_MASK_2)
    {
        dvec2 delta = new_ndc - prev_ndc;
        pan(delta);
    }
    else if (moveEvent.mask & BUTTON_MASK_3)
    {
        dvec2 delta = new_ndc - prev_ndc;
        zoom(delta.y);
    }

    prev_ndc = new_ndc;
    prev_tbc = new_tbc;
}

void Trackball::apply(ScrollWheelEvent& scrollWheel)
{
    zoom(scrollWheel.delta.y * 0.1);
}

void Trackball::apply(FrameEvent& /*frame*/)
{
    //    std::cout<<"Frame "<<frame.frameStamp->frameCount<<std::endl;
}

void Trackball::rotate(double angle, const dvec3& axis)
{
    dmat4 rotation = vsg::rotate(angle, axis);
    dmat4 lv = lookAt(_lookAt->eye, _lookAt->center, _lookAt->up);
    dvec3 centerEyeSpace = (lv * _lookAt->center);

    dmat4 matrix = inverse(lv) * translate(centerEyeSpace) * rotation * translate(-centerEyeSpace) * lv;

    _lookAt->up = normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
    _lookAt->center = matrix * _lookAt->center;
    _lookAt->eye = matrix * _lookAt->eye;
}

void Trackball::zoom(double ratio)
{
    dvec3 lookVector = _lookAt->center - _lookAt->eye;
    _lookAt->eye = _lookAt->eye + lookVector * ratio;
}

void Trackball::pan(dvec2& delta)
{
    dvec3 lookVector = _lookAt->center - _lookAt->eye;
    dvec3 lookNormal = normalize(lookVector);
    dvec3 sideNormal = cross(_lookAt->up, lookNormal);
    double distance = length(lookVector);
    dvec3 translation = sideNormal * (delta.x * distance) + _lookAt->up * (delta.y * distance);

    _lookAt->eye = _lookAt->eye + translation;
    _lookAt->center = _lookAt->center + translation;
}
