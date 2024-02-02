/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationManager.h>
#include <vsg/io/Options.h>

using namespace vsg;

AnimationManager::AnimationManager()
{
}

bool AnimationManager::start(vsg::ref_ptr<vsg::AnimationGroup> animationGroup, vsg::ref_ptr<vsg::Animation> animation)
{
    bool animationStarted = false;

    for(auto activeAnimation : animationGroup->active)
    {
        if (activeAnimation == animation)
        {
            vsg::info("AnimationManager::start(", animationGroup, ", ", animation, ", ", animation->name, ") can't start already active animnation.");
            return animationStarted;
        }
    }

    animationStarted = true;

    animationGroup->active.push_back(animation);

    animation->startTime = _simulationTime;

    bool animationGroupAlreadyAdded = false;
    for(auto ag : animationGroups)
    {
        if (ag == animationGroup) animationGroupAlreadyAdded = true;
    }

    if (!animationGroupAlreadyAdded)
    {
        animationGroups.push_back(animationGroup);
    }
    else
    {
        // vsg::info("AnimationManager::start(", animationGroup, ", ", animation, ", ", animation->name, ") can't start already active animnation group.");
    }

    vsg::info("AnimationManager::start(", animationGroup, ", ", animation, ", ", animation->name, ") added to active list");

    return animationStarted;
}

bool AnimationManager::end(vsg::ref_ptr<vsg::AnimationGroup> animationGroup, vsg::ref_ptr<vsg::Animation> animation)
{
    vsg::info("AnimationManager::end(", animationGroup, ", ", animation, ", ", animation->name, ")");

    bool animationEnded = false;

    for(auto a_itr = animationGroup->active.begin(); a_itr != animationGroup->active.end(); ++a_itr)
    {
        if ((*a_itr) == animation)
        {
            vsg::info("    removing Animation.");
            animationGroup->active.erase(a_itr);
            animationEnded = true;
            break;
        }
    }

    if (animationGroup->active.empty())
    {
        for(auto ag_itr = animationGroups.begin(); ag_itr != animationGroups.end(); ++ag_itr)
        {
            if ((*ag_itr) == animationGroup)
            {
                vsg::info("    removing AnimationGroup.");
                animationGroups.erase(ag_itr);
                animationEnded = true;
                break;
            }
        }
    }
    return animationEnded;
}

void AnimationManager::run(vsg::ref_ptr<vsg::FrameStamp> frameStamp)
{
    _simulationTime = frameStamp->simulationTime;

    for(auto ag : animationGroups)
    {
        ag->update(frameStamp->simulationTime);
    }
}

