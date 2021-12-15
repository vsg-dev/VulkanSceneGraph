/* <editor-fold desc="MIT License">

Copyright(c) 2019 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/viewer/Trackball.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

Trackball::Trackball(ref_ptr<Camera> camera, ref_ptr<EllipsoidModel> ellipsoidModel) :
    _camera(camera),
    _ellipsoidModel(ellipsoidModel)
{
    _lookAt = camera->viewMatrix.cast<LookAt>();

    if (!_lookAt)
    {
        // TODO: need to work out how to map the original ViewMatrix to a LookAt and back, for now just fallback to assigning our own LookAt
        _lookAt = new LookAt;
    }

    clampToGlobe();

    addKeyViewpoint(KEY_Space, LookAt::create(*_lookAt), 1.0);
}

void Trackball::clampToGlobe()
{
    //    std::cout<<"Trackball::clampToGlobe()"<<std::endl;

    if (!_ellipsoidModel) return;

    // get the location of the current lookAt center
    auto location_center = _ellipsoidModel->convertECEFToLatLongAltitude(_lookAt->center);
    auto location_eye = _ellipsoidModel->convertECEFToLatLongAltitude(_lookAt->eye);

    double ratio = location_eye.z / (location_eye.z - location_center.z);
    auto location = _ellipsoidModel->convertECEFToLatLongAltitude(_lookAt->center * ratio + _lookAt->eye * (1.0 - ratio));

    // clamp to the globe
    location.z = 0.0;

    // compute clamped position back in ECEF
    auto ecef = _ellipsoidModel->convertLatLongAltitudeToECEF(location);

    // apply the new clamped position to the LookAt.
    _lookAt->center = ecef;

    double minimum_altitude = 0.1;
    if (location_eye.z < minimum_altitude)
    {
        location_eye.z = minimum_altitude;
        _lookAt->eye = _ellipsoidModel->convertLatLongAltitudeToECEF(location_eye);
        _thrown = false;
    }
}

std::pair<int32_t, int32_t> Trackball::cameraRenderAreaCoordinates(const PointerEvent& pointerEvent) const
{
    if (!windowOffsets.empty())
    {
        auto itr = windowOffsets.find(pointerEvent.window);
        if (itr != windowOffsets.end())
        {
            auto& offset = itr->second;
            return {pointerEvent.x + offset.x, pointerEvent.y + offset.y};
        }
    }
    return {pointerEvent.x, pointerEvent.y};
}

bool Trackball::withinRenderArea(const PointerEvent& pointerEvent) const
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(pointerEvent);

    return (x >= renderArea.offset.x && x < static_cast<int32_t>(renderArea.offset.x + renderArea.extent.width)) &&
           (y >= renderArea.offset.y && y < static_cast<int32_t>(renderArea.offset.y + renderArea.extent.height));
}

/// compute non dimensional window coordinate (-1,1) from event coords
dvec2 Trackball::ndc(PointerEvent& event)
{
    auto renderArea = _camera->getRenderArea();
    auto [x, y] = cameraRenderAreaCoordinates(event);

    double aspectRatio = static_cast<double>(renderArea.extent.width) / static_cast<double>(renderArea.extent.height);
    dvec2 v(
        (renderArea.extent.width > 0) ? (static_cast<double>(x - renderArea.offset.x) / static_cast<double>(renderArea.extent.width) * 2.0 - 1.0) * aspectRatio : 0.0,
        (renderArea.extent.height > 0) ? static_cast<double>(y - renderArea.offset.y) / static_cast<double>(renderArea.extent.height) * 2.0 - 1.0 : 0.0);
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
    if (keyPress.handled || !_lastPointerEventWithinRenderArea) return;

    if (auto itr = keyViewpoitMap.find(keyPress.keyBase); itr != keyViewpoitMap.end())
    {
        _previousTime = keyPress.time;

        setViewpoint(itr->second.lookAt, itr->second.duration);

        keyPress.handled = true;
    }
}

void Trackball::apply(ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled) return;

    _hasFocus = withinRenderArea(buttonPress);
    _lastPointerEventWithinRenderArea = _hasFocus;

    if (buttonPress.mask & BUTTON_MASK_1)
        _updateMode = ROTATE;
    else if (buttonPress.mask & BUTTON_MASK_2)
        _updateMode = PAN;
    else if (buttonPress.mask & BUTTON_MASK_3)
        _updateMode = ZOOM;
    else
        _updateMode = INACTIVE;

    if (_hasFocus) buttonPress.handled = true;

    _zoomPreviousRatio = 0.0;
    _pan.set(0.0, 0.0);
    _rotateAngle = 0.0;

    _previousPointerEvent = &buttonPress;
}

void Trackball::apply(ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled) return;

    if (!windowOffsets.empty() && windowOffsets.count(buttonRelease.window) == 0) return;

    if (supportsThrow) _thrown = _previousPointerEvent && (buttonRelease.time == _previousPointerEvent->time);

    _lastPointerEventWithinRenderArea = withinRenderArea(buttonRelease);
    _hasFocus = false;

    _previousPointerEvent = &buttonRelease;
}

void Trackball::apply(MoveEvent& moveEvent)
{
    _lastPointerEventWithinRenderArea = withinRenderArea(moveEvent);

    if (moveEvent.handled || !_hasFocus) return;

    dvec2 new_ndc = ndc(moveEvent);
    dvec3 new_tbc = tbc(moveEvent);

    if (!_previousPointerEvent) _previousPointerEvent = &moveEvent;

    dvec2 prev_ndc = ndc(*_previousPointerEvent);
    dvec3 prev_tbc = tbc(*_previousPointerEvent);

#if 1
    dvec2 control_ndc = new_ndc;
    dvec3 control_tbc = new_tbc;
#else
    dvec2 control_ndc = (new_ndc + prev_ndc) * 0.5;
    dvec3 control_tbc = (new_tbc + prev_tbc) * 0.5;
#endif

    double dt = std::chrono::duration<double, std::chrono::seconds::period>(moveEvent.time - _previousPointerEvent->time).count();
    _previousDelta = dt;

    double scale = 1.0;
    //if (_previousTime > _previousPointerEvent->time) scale = std::chrono::duration<double, std::chrono::seconds::period>(moveEvent.time - _previousTime).count() / dt;
    //    scale *= 2.0;

    _previousTime = moveEvent.time;

    if (moveEvent.mask & rotateButtonMask)
    {
        _updateMode = ROTATE;

        moveEvent.handled = true;

        dvec3 xp = cross(normalize(control_tbc), normalize(prev_tbc));
        double xp_len = length(xp);
        if (xp_len > 0.0)
        {
            _rotateAngle = asin(xp_len);
            _rotateAxis = xp / xp_len;

            rotate(_rotateAngle * scale, _rotateAxis);
        }
        else
        {
            _rotateAngle = 0.0;
        }
    }
    else if (moveEvent.mask & panButtonMask)
    {
        _updateMode = PAN;

        moveEvent.handled = true;

        dvec2 delta = control_ndc - prev_ndc;

        _pan = delta;

        pan(delta * scale);
    }
    else if (moveEvent.mask & zoomButtonMask)
    {
        _updateMode = ZOOM;

        moveEvent.handled = true;

        dvec2 delta = control_ndc - prev_ndc;

        if (delta.y != 0.0)
        {
            _zoomPreviousRatio = zoomScale * 2.0 * delta.y;
            zoom(_zoomPreviousRatio * scale);
        }
    }

    _thrown = false;

    _previousPointerEvent = &moveEvent;
}

void Trackball::apply(ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled || !_lastPointerEventWithinRenderArea) return;

    scrollWheel.handled = true;

    zoom(scrollWheel.delta.y * 0.1);
}

void Trackball::apply(FrameEvent& frame)
{
    if (_endLookAt)
    {
        double timeSinceOfAnimation = std::chrono::duration<double, std::chrono::seconds::period>(frame.time - _startTime).count();
        if (timeSinceOfAnimation < _animationDuration)
        {
            double r = smoothstep(0.0, 1.0, timeSinceOfAnimation / _animationDuration);

            if (_ellipsoidModel)
            {
                auto interpolate = [](const dvec3& start, const dvec3& end, double ratio) -> dvec3 {
                    if (ratio >= 1.0) return end;

                    double length_start = length(start);
                    double length_end = length(end);
                    double acos_ratio = dot(start, end) / (length_start * length_end);
                    double angle = acos_ratio >= 1.0 ? 0.0 : (acos_ratio <= -1.0 ? vsg::PI : acos(acos_ratio));
                    auto cross_start_end = cross(start, end);
                    auto length_cross = length(cross_start_end);
                    if (angle != 0.0 && length_cross != 0.0)
                    {
                        cross_start_end /= length_cross;
                        auto rotation = vsg::rotate(angle * ratio, cross_start_end);
                        dvec3 new_dir = normalize(rotation * start);
                        return new_dir * mix(length_start, length_end, ratio);
                    }
                    else
                    {
                        return mix(start, end, ratio);
                    }
                };

                auto interpolate_arc = [](const dvec3& start, const dvec3& end, double ratio, double arc_height = 0.0) -> dvec3 {
                    if (ratio >= 1.0) return end;

                    double length_start = length(start);
                    double length_end = length(end);
                    double acos_ratio = dot(start, end) / (length_start * length_end);
                    double angle = acos_ratio >= 1.0 ? 0.0 : (acos_ratio <= -1.0 ? vsg::PI : acos(acos_ratio));
                    auto cross_start_end = cross(start, end);
                    auto length_cross = length(cross_start_end);
                    if (angle != 0.0 && length_cross != 0.0)
                    {
                        cross_start_end /= length_cross;
                        auto rotation = vsg::rotate(angle * ratio, cross_start_end);
                        dvec3 new_dir = normalize(rotation * start);
                        double target_length = mix(length_start, length_end, ratio) + (ratio - ratio * ratio) * arc_height * 4.0;
                        return new_dir * target_length;
                    }
                    else
                    {
                        return mix(start, end, ratio);
                    }
                };

                double length_center_start = length(_startLookAt->center);
                double length_center_end = length(_endLookAt->center);
                double length_center_mid = (length_center_start + length_center_end) * 0.5;
                double distance_between = length(_startLookAt->center - _endLookAt->center);

                double transition_length = length_center_mid + distance_between;

                double length_eye_start = length(_startLookAt->eye);
                double length_eye_end = length(_endLookAt->eye);
                double length_eye_mid = (length_eye_start + length_eye_end) * 0.5;

                double arc_height = (transition_length > length_eye_mid) ? (transition_length - length_eye_mid) : 0.0;

                _lookAt->eye = interpolate_arc(_startLookAt->eye, _endLookAt->eye, r, arc_height);
                _lookAt->center = interpolate(_startLookAt->center, _endLookAt->center, r);
                _lookAt->up = interpolate(_startLookAt->up, _endLookAt->up, r);
            }
            else
            {
                _lookAt->eye = mix(_startLookAt->eye, _endLookAt->eye, r);
                _lookAt->center = mix(_startLookAt->center, _endLookAt->center, r);

                double angle = acos(dot(_startLookAt->up, _endLookAt->up) / (length(_startLookAt->up) * length(_endLookAt->up)));
                if (angle != 0.0)
                {
                    auto rotation = vsg::rotate(angle * r, normalize(cross(_startLookAt->up, _endLookAt->up)));
                    _lookAt->up = rotation * _startLookAt->up;
                }
                else
                {
                    _lookAt->up = _endLookAt->up;
                }
            }
        }
        else
        {
            _lookAt->eye = _endLookAt->eye;
            _lookAt->center = _endLookAt->center;
            _lookAt->up = _endLookAt->up;

            _endLookAt = nullptr;
            _animationDuration = 0.0;
        }
    }
    else if (_thrown)
    {
        double scale = _previousDelta > 0.0 ? std::chrono::duration<double, std::chrono::seconds::period>(frame.time - _previousTime).count() / _previousDelta : 0.0;
        switch (_updateMode)
        {
        case (ROTATE):
            rotate(_rotateAngle * scale, _rotateAxis);
            break;
        case (PAN):
            pan(_pan * scale);
            break;
        case (ZOOM):
            zoom(_zoomPreviousRatio * scale);
            break;
        default:
            break;
        }
    }

    _previousTime = frame.time;
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

    clampToGlobe();
}

void Trackball::zoom(double ratio)
{
    dvec3 lookVector = _lookAt->center - _lookAt->eye;
    _lookAt->eye = _lookAt->eye + lookVector * ratio;

    clampToGlobe();
}

void Trackball::pan(const dvec2& delta)
{
    dvec3 lookVector = _lookAt->center - _lookAt->eye;
    dvec3 lookNormal = normalize(lookVector);
    dvec3 upNormal = _lookAt->up;
    dvec3 sideNormal = cross(lookNormal, upNormal);

    double distance = length(lookVector);
    distance *= 0.25; // TODO use Camera project matrix to guide how much to scale

    if (_ellipsoidModel)
    {
        double scale = distance;
        double angle = (length(delta) * scale) / _ellipsoidModel->radiusEquator();

        if (angle != 0.0)
        {
            dvec3 globeNormal = normalize(_lookAt->center);
            dvec3 m = upNormal * (-delta.y) + sideNormal * (delta.x); // compute the position relative to the center in the eye plane
            dvec3 v = m + lookNormal * dot(m, globeNormal);           // compensate for any tile relative to the globe normal
            dvec3 axis = normalize(cross(globeNormal, v));            // compute the axis of rotation to map the mouse pan

            dmat4 matrix = vsg::rotate(-angle, axis);

            _lookAt->up = normalize(matrix * (_lookAt->eye + _lookAt->up) - matrix * _lookAt->eye);
            _lookAt->center = matrix * _lookAt->center;
            _lookAt->eye = matrix * _lookAt->eye;

            clampToGlobe();
        }
    }
    else
    {
        dvec3 translation = sideNormal * (-delta.x * distance) + upNormal * (delta.y * distance);

        _lookAt->eye = _lookAt->eye + translation;
        _lookAt->center = _lookAt->center + translation;
    }
}

void Trackball::addWindow(ref_ptr<Window> window, const ivec2& offset)
{
    windowOffsets[observer_ptr<Window>(window)] = offset;
}

void Trackball::addKeyViewpoint(KeySymbol key, ref_ptr<LookAt> lookAt, double duration)
{
    keyViewpoitMap[key].lookAt = lookAt;
    keyViewpoitMap[key].duration = duration;
}

void Trackball::addKeyViewpoint(KeySymbol key, double latitude, double longitude, double altitude, double duration)
{
    if (!_ellipsoidModel) return;

    auto lookAt = LookAt::create();
    lookAt->eye = _ellipsoidModel->convertLatLongAltitudeToECEF(dvec3(latitude, longitude, altitude));
    lookAt->center = _ellipsoidModel->convertLatLongAltitudeToECEF(dvec3(latitude, longitude, 0.0));
    lookAt->up = normalize(cross(lookAt->center, dvec3(-lookAt->center.y, lookAt->center.x, 0.0)));

    keyViewpoitMap[key].lookAt = lookAt;
    keyViewpoitMap[key].duration = duration;
}

void Trackball::setViewpoint(ref_ptr<LookAt> lookAt, double duration)
{
    if (!lookAt) return;

    _thrown = false;

    if (duration == 0.0)
    {
        _lookAt->eye = lookAt->eye;
        _lookAt->center = lookAt->center;
        _lookAt->up = lookAt->up;

        _startLookAt = nullptr;
        _endLookAt = nullptr;
        _animationDuration = 0.0;

        clampToGlobe();
    }
    else
    {
        _startTime = _previousTime;
        _startLookAt = vsg::LookAt::create(*_lookAt);
        _endLookAt = lookAt;
        _animationDuration = duration;
    }
}
