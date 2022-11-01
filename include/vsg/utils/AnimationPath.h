#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/maths/quat.h>
#include <vsg/ui/KeyEvent.h>

#include <map>

namespace vsg
{

    /// AnimationPath class provides a container of path control points and
    /// animation via using the computeMatrix()/computeLocation() methods.
    class VSG_DECLSPEC AnimationPath : public Inherit<Object, AnimationPath>
    {
    public:
        enum Mode
        {
            ONCE,
            REPEAT,
            FORWARD_AND_BACK
        };

        struct Location
        {
            dvec3 position;
            dquat orientation;
            dvec3 scale = {1.0, 1.0, 1.0};
        };

        Mode mode = ONCE;
        std::map<double, Location> locations;

        void add(double time, const dvec3& position, const dquat& orientation = {}, const dvec3& scale = {1.0, 1.0, 1.0})
        {
            locations[time] = Location{position, orientation, scale};
        }

        double period() const;

        Location computeLocation(double time) const;
        dmat4 computeMatrix(double time) const;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::AnimationPath);

    /// AnimationPathHandler event handler animates Camera or MatrixTransform along an AnimationPath.
    /// To automatically update attach the AnimationPathHandler to the viewer using Viewer::addEventHandler().
    class VSG_DECLSPEC AnimationPathHandler : public Inherit<Visitor, AnimationPathHandler>
    {
    public:
        AnimationPathHandler(ref_ptr<Object> in_object, ref_ptr<AnimationPath> in_path, clock::time_point in_start_point);

        ref_ptr<Object> object;
        ref_ptr<AnimationPath> path;
        KeySymbol resetKey = KEY_Space;
        clock::time_point start_point;
        unsigned int frameCount = 0;
        double time = 0.0;
        bool printFrameStatsToConsole = false;

        void apply(Camera& camera) override;
        void apply(MatrixTransform& transform) override;

        void apply(KeyPressEvent& keyPress) override;
        void apply(FrameEvent& frame) override;

    protected:
    };
    VSG_type_name(vsg::AnimationPathHandler);

} // namespace vsg
