/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <istream>

namespace vsg
{

    /// read a line of space, tab or command delimited ascii values into array of ascii values.
    template<typename T>
    uint32_t read_line(std::istream& sstr, T* values, uint32_t maxSize, bool read_to_end_line = true)
    {
        uint32_t i = 0;
        auto c = sstr.peek();
        while (i < maxSize)
        {
            // skip leading spaces
            while (c == ' ' || c == '\t')
            {
                sstr.ignore();
                c = sstr.peek();
            }

            if (c == '#') break;

            if (read_to_end_line)
            {
                if (c == '\n')
                {
                    sstr.ignore();
                    return i;
                }
                else if (c == '\r')
                {
                    sstr.ignore();
                    c = sstr.peek();
                    if (c == '\n')
                    {
                        sstr.ignore();
                    }
                    return i;
                }
            }

            if (!(sstr >> values[i])) return i;

            ++i;

            // skip trailing spaces/tabs
            c = sstr.peek();

            while (c == ' ' || c == '\t')
            {
                sstr.ignore();
                c = sstr.peek();
            }

            // skip a single comma if we have one
            if (c == ',')
            {
                sstr.ignore();
                c = sstr.peek();
            }
        }

        if (read_to_end_line)
        {
            while (sstr && (c != '\n') && (c != '\r'))
            {
                sstr.ignore();
                c = sstr.peek();
            }

            if (c == '\n')
            {
                sstr.ignore();
            }
            else if (c == '\r')
            {
                sstr.ignore();
                c = sstr.peek();
                if (c == '\n')
                {
                    sstr.ignore();
                }
            }
        }

        return i;
    }

} // namespace vsg
