/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationManager.h>

using namespace vsg;

AnimationManager::AnimationManager()
{
}

void AnimationManager::assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation)
{
    instrumentation = in_instrumentation;
}

bool AnimationManager::play(vsg::ref_ptr<vsg::Animation> animation, double startTime)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "AnimationManager play animation", COLOR_VIEWER);

    bool already_active = animation->active();
    if (animation->start(_simulationTime, startTime))
    {
        if (!already_active) animations.push_back(animation);

        return true;
    }
    else
    {
        return false;
    }
}

bool AnimationManager::stop(vsg::ref_ptr<vsg::Animation> animation)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "AnimationManager stop animation", COLOR_VIEWER);

    auto itr = std::find(animations.begin(), animations.end(), animation);
    if (itr != animations.end())
    {
        animation->stop(_simulationTime);
        animations.erase(itr);
        return true;
    }
    else
    {
        return false;
    }
}

bool AnimationManager::stop()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "AnimationManager stop all animation", COLOR_VIEWER);

    for (auto& animation : animations)
    {
        animation->stop(_simulationTime);
    }
    animations.clear();
    return true;
}

bool AnimationManager::update(vsg::Animation& animation)
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "AnimationManager update animation", COLOR_VIEWER);
    return animation.update(_simulationTime);
}

void AnimationManager::run(vsg::ref_ptr<vsg::FrameStamp> frameStamp)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "AnimationManager run animation updates", COLOR_VIEWER);

    _simulationTime = frameStamp->simulationTime;

    for (auto itr = animations.begin(); itr != animations.end();)
    {
        if (update(**itr))
            ++itr;
        else
        {
            itr = animations.erase(itr);
        }
    }
}
