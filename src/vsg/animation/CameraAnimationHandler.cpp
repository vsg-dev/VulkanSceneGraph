/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/CameraAnimationHandler.h>
#include <vsg/animation/TransformSampler.h>
#include <vsg/app/Camera.h>
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PrintEvents.h>

using namespace vsg;

CameraAnimationHandler::CameraAnimationHandler()
{
}

CameraAnimationHandler::CameraAnimationHandler(ref_ptr<Object> in_object, const Path& in_filename, ref_ptr<Options> in_options) :
    object(in_object),
    filename(in_filename),
    options(in_options)
{
    if (filename)
    {
        if (auto read_object = vsg::read(filename, options))
        {
            if ((animation = read_object.cast<Animation>()))
            {
                for (auto sampler : animation->samplers)
                {
                    if (auto cs = sampler.cast<CameraSampler>())
                    {
                        cameraSampler = cs;
                        break;
                    }
                }
            }
            else if ((cameraSampler = read_object.cast<CameraSampler>()))
            {
                animation = Animation::create();
                animation->samplers.push_back(cameraSampler);
            }
            else if (auto ts = read_object.cast<TransformSampler>())
            {
                auto tkf = ts->keyframes;

                // convert TransformSampler to CameraSampler
                cameraSampler = CameraSampler::create();
                cameraSampler->name = ts->name;
                const auto& ckf = cameraSampler->keyframes = CameraKeyframes::create();

                ckf->positions = tkf->positions;
                ckf->rotations = tkf->rotations;

                animation = Animation::create();
                animation->samplers.push_back(cameraSampler);
            }
            else if (auto keyframes = read_object.cast<TransformKeyframes>())
            {
                cameraSampler = CameraSampler::create();
                const auto& ckf = cameraSampler->keyframes = CameraKeyframes::create();

                ckf->positions = keyframes->positions;
                ckf->rotations = keyframes->rotations;

                animation = Animation::create();
                animation->samplers.push_back(cameraSampler);
            }
        }
        if (object && cameraSampler) cameraSampler->object = object;
    }
    else
    {
        filename = "saved_animation.vsgt";
    }
}

CameraAnimationHandler::CameraAnimationHandler(ref_ptr<Object> in_object, ref_ptr<Animation> in_animation, const Path& in_filename, ref_ptr<Options> in_options) :
    object(in_object),
    filename(in_filename),
    options(in_options),
    animation(in_animation)
{
    if (animation)
    {
        for (auto& sampler : animation->samplers)
        {
            if (auto ts = sampler.cast<CameraSampler>())
            {
                cameraSampler = ts;
                cameraSampler->object = object;
                break;
            }
        }
    }
}

void CameraAnimationHandler::apply(Camera& camera)
{
    info("CameraAnimationHandler::apply(Camera& camera) ", cameraSampler);

    if (cameraSampler)
    {

        auto& keyframes = cameraSampler->keyframes;
        if (!keyframes) keyframes = CameraKeyframes::create();

        dvec3 position, scale;
        dquat orientation;
        auto matrix = camera.viewMatrix->inverse();
        if (decompose(matrix, position, orientation, scale))
        {
            keyframes->add(simulationTime - startTime, position, orientation);
        }
    }
}

void CameraAnimationHandler::apply(MatrixTransform& transform)
{
    if (cameraSampler)
    {
        auto& keyframes = cameraSampler->keyframes;
        if (!keyframes) keyframes = CameraKeyframes::create();

        dvec3 position, scale;
        dquat orientation;
        if (decompose(transform.matrix, position, orientation, scale))
        {
            keyframes->add(simulationTime - startTime, position, orientation);
        }
    }
}

void CameraAnimationHandler::play()
{
    if (playing) return;

    playing = animation->start(simulationTime);
    if (playing) info("Starting playback.");
}

void CameraAnimationHandler::record()
{
    if (recording) return;

    info("Starting recording.");
    startTime = simulationTime;
    recording = true;

    if (!animation)
    {
        animation = Animation::create();
    }
    if (!cameraSampler)
    {
        cameraSampler = CameraSampler::create();
        cameraSampler->object = object;

        animation->samplers.push_back(cameraSampler);
    }

    if (cameraSampler->keyframes)
    {
        cameraSampler->keyframes->clear();
    }
    else
    {
        cameraSampler->keyframes = CameraKeyframes::create();
    }
}

void CameraAnimationHandler::stop()
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

void CameraAnimationHandler::apply(KeyPressEvent& keyPress)
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

void CameraAnimationHandler::apply(FrameEvent& frame)
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
