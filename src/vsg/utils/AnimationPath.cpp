/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/utils/AnimationPath.h>

using namespace vsg;

double AnimationPath::period() const
{
    if (locations.empty()) return 0.0;
    return locations.rbegin()->first - locations.begin()->first;
}

AnimationPath::Location AnimationPath::computeLocation(double time) const
{
    // check for empy locations map
    if (locations.empty()) return {};

    // check for single entry in locations map
    if (locations.begin() == locations.rbegin().base()) return locations.begin()->second;

    if (mode == REPEAT)
    {
        time = locations.begin()->first + std::fmod(time - locations.begin()->first, period());
    }
    else if (mode == FORWARD_AND_BACK)
    {
        double p = period();
        double t = std::fmod(time - locations.begin()->first, p * 2.0);
        if (t <= p)
            time = locations.begin()->first + t;
        else if (t > p)
            time = locations.begin()->first + p * 2.0 - t;
    }

    if (time <= locations.begin()->first) return locations.begin()->second;
    if (time >= locations.rbegin()->first) return locations.rbegin()->second;

    auto not_less_itr = locations.lower_bound(time);
    if (not_less_itr == locations.end()) return {};
    if (not_less_itr == locations.begin()) return not_less_itr->second;

    auto less_than_itr = not_less_itr;
    --not_less_itr;

    auto& lower = less_than_itr->second;
    auto& upper = not_less_itr->second;
    double r = (time - less_than_itr->first) / (not_less_itr->first - less_than_itr->first);

    return Location{mix(lower.position, upper.position, r), mix(lower.orientation, upper.orientation, r)};
}
