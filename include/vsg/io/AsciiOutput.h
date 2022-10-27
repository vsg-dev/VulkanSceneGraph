#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

#include <algorithm>
#include <fstream>

namespace vsg
{

    /// vsg::Output subclass that implements writing objects as ascii data to an output stream.
    /// Used by VSG ReaderWriter when writing objects to native .vsgt ascii files.
    class VSG_DECLSPEC AsciiOutput : public vsg::Output
    {
    public:
        explicit AsciiOutput(std::ostream& output, ref_ptr<const Options> in_options = {});

        std::ostream& indent()
        {
            _output.write(_indentationString, std::min(_indentation, _maximumIndentation));
            return _output;
        }

        /// write property name if appropriate for format
        void writePropertyName(const char* propertyName) override;

        /// write end of line as an \n
        void writeEndOfLine() override { _output << '\n'; }

        template<typename T>
        void _write(size_t num, const T* value)
        {
            if (num == 1)
            {
                _output << ' ' << *value;
            }
            else
            {
                for (size_t numInRow = 1; num > 0; --num, ++value, ++numInRow)
                {
                    _output << ' ' << *value;

                    if (numInRow == _maximumNumbersPerLine && num > 1)
                    {
                        numInRow = 0;
                        writeEndOfLine();
                        indent();
                    }
                }
            }
        }

        template<typename T>
        void _write_real(size_t num, const T* value)
        {
            if (num == 1)
            {
                if (std::isfinite(*value))
                    _output << ' ' << *value;
                else
                    _output << ' ' << 0.0; // fallback to using 0.0 when the value is NaN or Infinite to prevent problems when reading
            }
            else
            {
                for (size_t numInRow = 1; num > 0; --num, ++value, ++numInRow)
                {
                    if (std::isfinite(*value))
                        _output << ' ' << *value;
                    else
                        _output << ' ' << 0.0; // fallback to using 0.0 when the value is NaN or Infinite to prevent problems when reading

                    if (numInRow == _maximumNumbersPerLine && num > 1)
                    {
                        numInRow = 0;
                        writeEndOfLine();
                        indent();
                    }
                }
            }
        }

        template<typename R, typename T>
        void _write_withcast(size_t num, const T* value)
        {
            if (num == 1)
            {
                _output << ' ' << static_cast<R>(*value);
            }
            else
            {
                for (size_t numInRow = 1; num > 0; --num, ++value, ++numInRow)
                {
                    _output << ' ' << static_cast<R>(*value);

                    if (numInRow == _maximumNumbersPerLine && num > 1)
                    {
                        numInRow = 0;
                        writeEndOfLine();
                        indent();
                    }
                }
            }
        }

        // write contiguous array of value(s)
        void write(size_t num, const int8_t* value) override { _write_withcast<int16_t>(num, value); }
        void write(size_t num, const uint8_t* value) override { _write_withcast<uint16_t>(num, value); }

        void write(size_t num, const int16_t* value) override { _write(num, value); }
        void write(size_t num, const uint16_t* value) override { _write(num, value); }
        void write(size_t num, const int32_t* value) override { _write(num, value); }
        void write(size_t num, const uint32_t* value) override { _write(num, value); }
        void write(size_t num, const int64_t* value) override { _write(num, value); }
        void write(size_t num, const uint64_t* value) override { _write(num, value); }
        void write(size_t num, const float* value) override { _write_real(num, value); }
        void write(size_t num, const double* value) override { _write_real(num, value); }

        void _write(const std::string& str)
        {
            _output << '"';
            for (auto c : str)
            {
                if (c == '"')
                    _output << "\\\"";
                else
                    _output << c;
            }
            _output << '"';
        }

        void write(size_t num, const std::string* value) override;
        void write(size_t num, const Path* value) override;

        /// write object
        void write(const vsg::Object* object) override;

    protected:
        std::ostream& _output;

        std::size_t _indentationStep = 2;
        std::size_t _indentation = 0;
        std::size_t _maximumIndentation = 0;
        std::size_t _maximumNumbersPerLine = 12;
        // 24 characters long enough for 12 levels of nesting
        const char* _indentationString = "                        ";
    };

} // namespace vsg
