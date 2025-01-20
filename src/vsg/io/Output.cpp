/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

using namespace vsg;

Output::Output() :
    version{vsgGetVersion()}
{
    objectIDMap[nullptr] = 0;
}

Output::Output(ref_ptr<const Options> in_options) :
    Output()
{
    options = in_options;
}

Output::~Output()
{
}

bool Output::version_less(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion) const
{
    return version < VsgVersion{major, minor, patch, soversion};
}

bool Output::version_greater_equal(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion) const
{
    return !version_less(major, minor, patch, soversion);
}
