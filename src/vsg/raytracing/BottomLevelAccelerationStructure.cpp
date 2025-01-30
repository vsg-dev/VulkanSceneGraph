/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/raytracing/BottomLevelAccelerationStructure.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(Device* device) :
    Inherit(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, device)
{
}

void BottomLevelAccelerationStructure::compile(Context& context)
{
    if (geometries.empty()) return;                        // no data
    if (_vkGeometries.size() == geometries.size()) return; // already compiled

    for (auto& geom : geometries)
    {
        geom->compile(context);
        _vkGeometries.push_back(*geom);
    }

    // set the additional acceleration structure info used in the base AccelerationStructure compile function
    for (const auto& geom : geometries)
    {
        _geometryPrimitiveCounts.push_back(static_cast<uint32_t>(geom->indices->valueCount() / 3));
    }
    _accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(geometries.size());
    _accelerationStructureBuildGeometryInfo.pGeometries = _vkGeometries.data();

    Inherit::compile(context);

    context.buildAccelerationStructureCommands.push_back(BuildAccelerationStructureCommand::create(context.device, _accelerationStructureBuildGeometryInfo, _accelerationStructure, _geometryPrimitiveCounts));
}
