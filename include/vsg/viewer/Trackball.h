/* <editor-fold desc="MIT License">

Copyright(c) 2019 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/transform.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/Camera.h>

namespace vsg
{
    template<typename T>
    constexpr t_mat4<T> lookAtInverse(t_vec3<T> const& eye, t_vec3<T> const& center, t_vec3<T> const& up)
    {
        using vec_type = t_vec3<T>;

        vec_type forward = normalize(center - eye);
        vec_type up_normal = normalize(up);
        vec_type side = normalize(cross(forward, up_normal));
        vec_type u = normalize(cross(side, forward));

        return translate(eye.x, eye.y, eye.z) *
            t_mat4<T>(side[0], u[0], -forward[0], 0,
                            side[1], u[1], -forward[1], 0,
                            side[2], u[2], -forward[2], 0,
                            0, 0, 0, 1);
    }

    class Trackball : public Inherit<Visitor, Trackball>
    {
    public:

        Trackball(ref_ptr<Camera> camera);

        /// compute non dimensional window coordinate (-1,1) from event coords
        dvec2 ndc(PointerEvent& event);

        /// compute trackball coordinate from event coords
        dvec3 tbc(PointerEvent& event);

        void apply(KeyPressEvent& keyPress) override;
        void apply(ConfigureWindowEvent& exposeWindow) override;
        void apply(ButtonPressEvent& buttonPress) override;
        void apply(ButtonReleaseEvent& buttonRelease) override;
        void apply(MoveEvent& moveEvent) override;
        void apply(FrameEvent& frame) override;

        void rotate(double angle, const dvec3& axis);
        void zoom(double ratio);
        void pan(dvec2& delta);

    protected:
        ref_ptr<Camera> _camera;
        ref_ptr<LookAt> _lookAt;
        ref_ptr<LookAt> _homeLookAt;

        KeySymbol _homeKey = KEY_Space;
        double _direction;

        double window_width = 800.0;
        double window_height = 600.0;
        dvec2 prev_ndc;
        dvec3 prev_tbc;
    };

}

