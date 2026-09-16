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
    // make sure to have a default VkPhysicalDeviceFeatures2 created
    get();
    // Specs state:
    // "For all features, including the Core 1.0 Features, use VkPhysicalDeviceFeatures2 to pass into VkDeviceCreateInfo.pNext"
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
    // chain the Feature pNext pointers together
    // IF NOTHING WAS SETUP - this will return nullptr as well.
    FeatureHeader *previous = nullptr, *first = nullptr;
    for (auto it : _features)
        if (it.second.first->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2)
            first = it.second.first;
        else
        {
            it.second.first->pNext = previous;
            previous = it.second.first;
        }
    if (first)
        first->pNext = previous;
    else // Fallback path: try to simply hand over, what we have, if any …
        first = previous;

    // return head of the chain
    return const_cast<void*>(reinterpret_cast<const void*>(first));
}
