/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationHandler.h>
#include <vsg/app/Camera.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PrintEvents.h>

using namespace vsg;


AnimationHandler::AnimationHandler(ref_ptr<Object> in_object, const Path& in_filename, ref_ptr<Options> in_options) :
    object(in_object),
    filename(in_filename),
    options(in_options)
{
    if (filename)
    {
        if (auto read_object = vsg::read(filename, options))
        {
            if (animation = read_object.cast<Animation>())
            {
                for(auto sampler : animation->samplers)
                {
                    if (auto ts = sampler.cast<TransformSampler>())
                    {
                        transformSampler = ts;
                        break;
                    }
                }
            }
            else if (transformSampler = read_object.cast<TransformSampler>())
            {
                animation = Animation::create();
                animation->samplers.push_back(transformSampler);
            }
            else if (auto keyframes = read_object.cast<TransformKeyframes>())
            {
                transformSampler = TransformSampler::create();
                transformSampler->keyframes = keyframes;

                animation = Animation::create();
                animation->samplers.push_back(transformSampler);
            }
        }
        if (object && transformSampler) transformSampler->object = object;
    }
    else
    {
        filename = "saved_animation.vsgt";
    }
}

void AnimationHandler::apply(Camera& camera)
{
    if (transformSampler)
    {
        auto& keyframes = transformSampler->keyframes;
        if (!keyframes) keyframes = TransformKeyframes::create();

        dvec3 position, scale;
        dquat orientation;
        auto matrix = camera.viewMatrix->inverse();
        if (decompose(matrix, position, orientation, scale))
        {
            keyframes->add(simulationTime - startTime, position, orientation, scale);
        }
    }
}

void AnimationHandler::apply(MatrixTransform& transform)
{
    if (transformSampler)
    {
        auto& keyframes = transformSampler->keyframes;
        if (!keyframes) keyframes = TransformKeyframes::create();

        dvec3 position, scale;
        dquat orientation;
        if (decompose(transform.matrix, position, orientation, scale))
        {
            keyframes->add(simulationTime - startTime, position, orientation, scale);
        }
    }
}

void AnimationHandler::play()
{
    if (playing) return;

    playing = animation->start(simulationTime);
    if (playing) info("Starting playback.");
}

void AnimationHandler::record()
{
    if (recording) return;

    info("Starting recording.");
    startTime = simulationTime;
    recording = true;

    if (!animation)
    {
        animation = Animation::create();
    }
    if (!transformSampler)
    {
        transformSampler = TransformSampler::create();
        transformSampler->object = object;

        animation->samplers.push_back(transformSampler);
    }

    if (transformSampler->keyframes)
    {
        transformSampler->keyframes->clear();
    }
    else
    {
        transformSampler->keyframes = TransformKeyframes::create();
    }
}

void AnimationHandler::stop()
{
    if (playing)
    {
        info("Stopping playback.");
        playing = animation->stop(simulationTime);
    }
    else if (recording)
    {
        info("Stop recording.");

        if (filename)
        {
            if (vsg::write(animation, filename, options))
            {
                info("Written recoded path to : ", filename);
            }
        }

        recording = false;
        playing = false;
    }
}

void AnimationHandler::apply(KeyPressEvent& keyPress)
{
    if (keyPress.keyModified == togglePlaybackKey)
    {
        recording = false;
        if (animation)
        {
            if (!playing)
            {
                play();
            }
            else
            {
                stop();
            }
        }
    }
    else if (keyPress.keyModified == toggleRecordingKey)
    {
        if (!recording)
        {
            record();
        }
        else
        {
            stop();
        }
    }
}

void AnimationHandler::apply(FrameEvent& frame)
{
    simulationTime = frame.frameStamp->simulationTime;

    if (!object) return;

    if (playing)
    {
        animation->update(simulationTime);
    }
    else if (recording)
    {
        object->accept(*this);
    }
}
