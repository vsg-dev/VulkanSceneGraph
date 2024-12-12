#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/maths/transform.h>

namespace vsg
{

    template<typename T>
    struct time_value
    {
        using value_type = T;
        double time;
        value_type value;

        bool operator<(const time_value& rhs) const { return time < rhs.time; }
    };

    using time_double = time_value<double>;
    using time_dvec2 = time_value<dvec2>;
    using time_dvec3 = time_value<dvec3>;
    using time_dvec4 = time_value<dvec4>;
    using time_dquat = time_value<dquat>;

    template<typename T, typename V>
    bool sample(double time, const T& values, V& value)
    {
        if (values.size() == 0) return false;

        if (values.size() == 1)
        {
            value = values.front().value;
            return true;
        }

        auto pos_itr = values.begin();
        if (time <= pos_itr->time)
        {
            value = pos_itr->value;
            return true;
        }
        else
        {
            using value_type = typename T::value_type;
            pos_itr = std::lower_bound(values.begin(), values.end(), time, [](const value_type& elem, double t) -> bool { return elem.time < t; });

            if (pos_itr == values.begin())
            {
                value = values.front().value;
                return true;
            }

            if (pos_itr == values.end())
            {
                value = values.back().value;
                return true;
            }

            auto before_pos_itr = pos_itr - 1;
            double delta_time = (pos_itr->time - before_pos_itr->time);
            double r = delta_time != 0.0 ? (time - before_pos_itr->time) / delta_time : 0.5;

            value = mix(before_pos_itr->value, pos_itr->value, r);

            return true;
        }
    }


} // namespace vsg
