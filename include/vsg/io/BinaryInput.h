#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>

#include <vsg/io/Input.h>
#include <vsg/io/Options.h>

#include <fstream>

namespace vsg
{

    /// vsg::Input subclass that implements reading from a binary input stream.
    /// Used by VSG ReaderWriter when reading native .vsgb binary files.
    class VSG_DECLSPEC BinaryInput : public vsg::Input
    {
    public:
        BinaryInput(std::istream& input, ref_ptr<ObjectFactory> in_objectFactory, ref_ptr<const Options> in_options = {});

        bool matchPropertyName(const char*) override { return true; }

        ObjectID objectID()
        {
            ObjectID id;
            _input.read(reinterpret_cast<char*>(&id), sizeof(uint32_t));
            return id;
        }

        template<typename T>
        void _read(size_t num, T* value)
        {
            _input.read(reinterpret_cast<char*>(value), num * sizeof(T));
        }

        // read value(s)
        void read(size_t num, int8_t* value) override { _read(num, value); }
        void read(size_t num, uint8_t* value) override { _read(num, value); }
        void read(size_t num, int16_t* value) override { _read(num, value); }
        void read(size_t num, uint16_t* value) override { _read(num, value); }
        void read(size_t num, int32_t* value) override { _read(num, value); }
        void read(size_t num, uint32_t* value) override { _read(num, value); }
        void read(size_t num, int64_t* value) override { _read(num, value); }
        void read(size_t num, uint64_t* value) override { _read(num, value); }
        void read(size_t num, float* value) override { _read(num, value); }
        void read(size_t num, double* value) override { _read(num, value); }

        // read in an individual string
        void _read(std::string& value);

        /// read one or more strings
        void read(size_t num, std::string* value) override;

        /// read one or more paths
        void read(size_t num, Path* value) override;

        /// read object
        vsg::ref_ptr<vsg::Object> read() override;

    protected:
        std::istream& _input;
    };

} // namespace vsg
