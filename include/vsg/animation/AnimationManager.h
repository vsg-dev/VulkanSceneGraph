#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/ui/FrameStamp.h>

namespace vsg
{

    /// AnimationManager provides a mechanism for playing/updating animations as part of the Viewer::update()
    class VSG_DECLSPEC AnimationManager : public vsg::Inherit<Object, AnimationManager>
    {
    public:
        AnimationManager();

        /// list of animations that are currently being played
        std::list<ref_ptr<Animation>> animations;

        ref_ptr<Instrumentation> instrumentation;

        /// assign instrumentation if required
        virtual void assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation);

        /// play animation
        virtual bool play(vsg::ref_ptr<vsg::Animation> animation);

        /// stop animation
        virtual bool stop(vsg::ref_ptr<vsg::Animation> animation);

        /// stop all running animations
        virtual bool stop();

        /// update animation, called automatically by AnimationManager::run()
        virtual bool update(vsg::Animation& animation);

        /// update all the animations being played, called automatically by Viewer::update()
        virtual void run(vsg::ref_ptr<vsg::FrameStamp> frameStamp);

    protected:
        double _simulationTime = 0.0;
    };
    VSG_type_name(vsg::AnimationManager);

} // namespace vsg
