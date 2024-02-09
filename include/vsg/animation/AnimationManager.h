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

    class VSG_DECLSPEC AnimationManager : public vsg::Inherit<Object, AnimationManager>
    {
    public:
        AnimationManager();

        Animations animations;

        ref_ptr<Instrumentation> instrumentation;

        virtual void assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation);

        virtual bool start(vsg::ref_ptr<vsg::Animation> animation);

        virtual bool end(vsg::ref_ptr<vsg::Animation> animation);

        virtual void run(vsg::ref_ptr<vsg::FrameStamp> frameStamp);

    protected:
        double _simulationTime = 0.0;
    };
    VSG_type_name(vsg::AnimationManager);

} // namespace vsg
