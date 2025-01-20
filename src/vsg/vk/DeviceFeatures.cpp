/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DeviceFeatures.h>

using namespace vsg;

DeviceFeatures::DeviceFeatures()
{
}

DeviceFeatures::~DeviceFeatures()
{
    clear();
}

VkPhysicalDeviceFeatures& DeviceFeatures::get()
{
    return get<VkPhysicalDeviceFeatures2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2>().features;
}

void DeviceFeatures::clear()
{
    for (auto& feature : _features)
    {
        feature.second.second(feature.second.first);
    }

    _features.clear();
}

void* DeviceFeatures::data() const
{
    if (_features.empty()) return nullptr;

    // chain the Feature pNext pointers together
    FeatureHeader* previous = nullptr;
    for (auto itr = _features.rbegin(); itr != _features.rend(); ++itr)
    {
        itr->second.first->pNext = previous;
        previous = itr->second.first;
    }

    // return head of the chain
    return const_cast<void*>(reinterpret_cast<const void*>(_features.begin()->second.first));
}
