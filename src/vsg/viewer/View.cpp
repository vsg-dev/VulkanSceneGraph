/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/nodes/Bin.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

// thread safe container for managing the deviceID for each vsg;:View
static std::mutex s_ViewCountMutex;
static std::vector<bool> s_ActiveViews;

static uint32_t getUniqueViewID()
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);

    uint32_t viewID = 0;
    for (viewID = 0; viewID < static_cast<uint32_t>(s_ActiveViews.size()); ++viewID)
    {
        if (!s_ActiveViews[viewID])
        {
            s_ActiveViews[viewID] = true;
            return viewID;
        }
    }

    s_ActiveViews.push_back(true);

    return viewID;
}

static void releaseViewID(uint32_t viewID)
{
    std::scoped_lock<std::mutex> guard(s_ViewCountMutex);
    s_ActiveViews[viewID] = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewDependentState
//
ViewDependentState::ViewDependentState(uint32_t maxNumberLights)
{
    lightData = vec4Array::create(maxNumberLights);
    lightDescriptor = DescriptorImage::create();
    descriptorSet = DescriptorSet::create();
    bindDescriptorSet = BindDescriptorSet::create();
}

ViewDependentState::~ViewDependentState()
{
}

void ViewDependentState::clear()
{
    std::cout<<"ViewDependentState::clear()"<<std::endl;
    ambientLights.clear();
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();
}

void ViewDependentState::push(State& state)
{
    std::cout<<"ViewDependentState::push()"<<std::endl;
    if (bindDescriptorSet)
    {
        state.stateStacks[bindDescriptorSet->slot].push(bindDescriptorSet);
        state.dirty = true;
    }
}

void ViewDependentState::pop(State& state)
{
    std::cout<<"ViewDependentState::pop()"<<std::endl;
    if (bindDescriptorSet)
    {
        state.stateStacks[bindDescriptorSet->slot].pop();
        state.dirty = true;
    }
}

void ViewDependentState::pack()
{
    std::cout<<"ViewDependentState::pack()"<<std::endl;
    auto light_itr = lightData->begin();

    (*light_itr++) = vec4(static_cast<float>(ambientLights.size()),
                    static_cast<float>(directionalLights.size()),
                    static_cast<float>(pointLights.size()),
                    static_cast<float>(spotLights.size()));

    for(auto& [mv, light] : ambientLights)
    {
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
    }

    for(auto& [mv, light] : directionalLights)
    {
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(eye_direction.x, eye_direction.y, eye_direction.z, 0.0f);
    }

    for(auto& [mv, light] : pointLights)
    {
        auto eye_position = mv * light->position;
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(eye_position.x, eye_position.y, eye_position.z, 0.0f);
    }

    for(auto& [mv, light] : spotLights)
    {
        auto eye_position = mv * light->position;
        auto eye_direction = normalize(light->direction * inverse_3x3(mv));
        (*light_itr++).set(light->color.r, light->color.g, light->color.b, light->intensity);
        (*light_itr++).set(eye_position.x, eye_position.y, eye_position.z, 0.0f);
        (*light_itr++).set(eye_direction.x, eye_direction.y, eye_direction.z, 0.0f);
    }

    for(auto itr = lightData->begin(); itr != light_itr; ++itr)
    {
        std::cout<<"   "<<*itr<<std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// View
//
View::View() :
    viewID(getUniqueViewID())
{
    viewDependentState = ViewDependentState::create();
}

View::View(ref_ptr<Camera> in_camera, ref_ptr<Node> in_scenegraph) :
    viewID(getUniqueViewID())
{
    viewDependentState = ViewDependentState::create();
    camera = in_camera;

    if (in_scenegraph) addChild(in_scenegraph);
}

View::~View()
{
    releaseViewID(viewID);
}
