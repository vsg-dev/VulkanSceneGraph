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
#include <vsg/vk/Extensions.h>

using namespace vsg;

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(Device* device, Allocator* allocator) :
    Inherit(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV, device, allocator)
{
}

void BottomLevelAccelerationStructure::compile(Context& context)
{
    if (_geometries.size() == 0) return;                    // no data
    if (_vkGeometries.size() == _geometries.size()) return; // already compiled

    for (auto& geom : _geometries)
    {
        geom->compile(context);
        _vkGeometries.push_back(*geom);
    }

    // set the aditional acceleration structure info used in the base AccelerationStructure compile function
    _accelerationStructureInfo.geometryCount = static_cast<uint32_t>(_geometries.size());
    _accelerationStructureInfo.pGeometries = _vkGeometries.data();

    Inherit::compile(context);

    context.buildAccelerationStructureCommands.push_back(BuildAccelerationStructureCommand::create(context.device, &_accelerationStructureInfo, _accelerationStructure, nullptr));
}

void BottomLevelAccelerationStructure::dispatch(CommandBuffer& commandBuffer) const
{
}
