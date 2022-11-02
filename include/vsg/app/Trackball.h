#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/app/EllipsoidModel.h>
#include <vsg/maths/transform.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/TouchEvent.h>

namespace vsg
{

    /// Trackball is an event handler that provides mouse and touch controlled 3d trackball camera view manipulation.
    class VSG_DECLSPEC Trackball : public Inherit<Visitor, Trackball>
    {
    public:
        explicit Trackball(ref_ptr<Camera> camera, ref_ptr<EllipsoidModel> ellipsoidModel = {});

        /// compute non dimensional window coordinate (-1,1) from event coords
        dvec2 ndc(PointerEvent& event);

        /// compute trackball coordinate from event coords
        dvec3 tbc(PointerEvent& event);

        void apply(KeyPressEvent& keyPress) override;
        void apply(ButtonPressEvent& buttonPress) override;
        void apply(ButtonReleaseEvent& buttonRelease) override;
        void apply(MoveEvent& moveEvent) override;
        void apply(ScrollWheelEvent& scrollWheel) override;
        void apply(TouchDownEvent& touchDown) override;
        void apply(TouchUpEvent& touchUp) override;
        void apply(TouchMoveEvent& touchMove) override;
        void apply(FrameEvent& frame) override;

        virtual void rotate(double angle, const dvec3& axis);
        virtual void zoom(double ratio);
        virtual void pan(const dvec2& delta);

        std::pair<int32_t, int32_t> cameraRenderAreaCoordinates(const PointerEvent& pointerEvent) const;
        bool withinRenderArea(const PointerEvent& pointerEvent) const;

        void clampToGlobe();

        /// list of windows that this Trackball should respond to events from, and the points xy offsets to apply
        std::map<observer_ptr<Window>, ivec2> windowOffsets;

        /// add a Window to respond events for, with mouse coordinate offset to treat all associated windows
        void addWindow(ref_ptr<Window> window, const ivec2& offset = {});

        /// add Key to Viewpoint binding using a LookAt to define the viewpoint
        void addKeyViewpoint(KeySymbol key, ref_ptr<LookAt> lookAt, double duration = 1.0);

        /// add Key to Viewpoint binding using a latitude, longitude and altitude to define the viewpoint. Requires an EllipsoidModel to be assigned when constructing the Trackball
        void addKeyViewpoint(KeySymbol key, double latitude, double longitude, double altitude, double duration = 1.0);

        /// set the LookAt viewport to the specified lookAt, animating the movements from the current lookAt to the new one.
        /// A value of 0.0 instantly moves the lookAt to the new value.
        void setViewpoint(ref_ptr<LookAt> lookAt, double duration = 1.0);

        struct Viewpoint
        {
            ref_ptr<LookAt> lookAt;
            double duration = 0.0;
        };

        /// container that maps key symbol bindings with the Viewpoint that should move the LookAt to when pressed.
        std::map<KeySymbol, Viewpoint> keyViewpoitMap;

        /// Button mask value used to enable panning of the view, defaults to left mouse button
        ButtonMask rotateButtonMask = BUTTON_MASK_1;

        /// Button mask value used to enable panning of the view, defaults to middle mouse button
        ButtonMask panButtonMask = BUTTON_MASK_2;

        /// Button mask value used to enable zooming of the view, defaults to right mouse button
        ButtonMask zoomButtonMask = BUTTON_MASK_3;

        /// Scale for control how rapidly the view zooms in/out. Positive value zooms in when mouse moved downwards
        double zoomScale = 1.0;

        /// Toggle on/off whether the view should continue moving when the mouse buttons are released while the mouse is in motion.
        bool supportsThrow = true;

    protected:
        ref_ptr<Camera> _camera;
        ref_ptr<LookAt> _lookAt;
        ref_ptr<EllipsoidModel> _ellipsoidModel;

        bool _hasFocus = false;
        bool _lastPointerEventWithinRenderArea = false;

        enum UpdateMode
        {
            INACTIVE = 0,
            ROTATE,
            PAN,
            ZOOM
        };
        UpdateMode _updateMode = INACTIVE;
        double _zoomPreviousRatio = 0.0;
        dvec2 _pan;
        double _rotateAngle = 0.0;
        dvec3 _rotateAxis;

        time_point _previousTime;
        ref_ptr<PointerEvent> _previousPointerEvent;
        double _previousDelta = 0.0;
        double _prevZoomTouchDistance = 0.0;
        bool _thrown = false;

        time_point _startTime;
        ref_ptr<LookAt> _startLookAt;
        ref_ptr<LookAt> _endLookAt;
        std::map<uint32_t, ref_ptr<TouchEvent>> _previousTouches;

        double _animationDuration = 0.0;
    };
    VSG_type_name(vsg::Trackball);

} // namespace vsg
