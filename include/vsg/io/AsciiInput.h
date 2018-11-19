#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>

#include <vsg/io/Input.h>
#include <vsg/io/ObjectFactory.h>

#include <fstream>
#include <optional>
#include <unordered_map>

namespace vsg
{

    class VSG_DECLSPEC AsciiInput : public vsg::Input
    {
    public:
        using ObjectID = uint32_t;

        AsciiInput(std::istream& input);

        bool matchPropertyName(const char* propertyName) override;

        std::optional<ObjectID> objectID();

        template<typename T>
        void _read(size_t num, T* value)
        {
            if (num == 1)
            {
                _input >> *value;
            }
            else
            {
                for (; num > 0; --num, ++value)
                {
                    _input >> *value;
                }
            }
        }

        // read value(s)
        virtual void read(size_t num, int8_t* value) override { _read(num, value); }
        virtual void read(size_t num, uint8_t* value) override { _read(num, value); }
        virtual void read(size_t num, int16_t* value) override { _read(num, value); }
        virtual void read(size_t num, uint16_t* value) override { _read(num, value); }
        virtual void read(size_t num, int32_t* value) override { _read(num, value); }
        virtual void read(size_t num, uint32_t* value) override { _read(num, value); }
        virtual void read(size_t num, int64_t* value) override { _read(num, value); }
        virtual void read(size_t num, uint64_t* value) override { _read(num, value); }
        virtual void read(size_t num, float* value) override { _read(num, value); }
        virtual void read(size_t num, double* value) override { _read(num, value); }

        // read in an individual string
        void _read(std::string& value);

        // read one or more strings
        void read(size_t num, std::string* value) override;

        // read object
        vsg::ref_ptr<vsg::Object> read() override;

    protected:
        std::istream& _input;

#if 0
        using ObjectIDMap = std::map<ObjectID, vsg::ref_ptr<vsg::Object>>;
#else
        // 47% faster for overall write for large scene graph than std::map<>!
        using ObjectIDMap = std::unordered_map<ObjectID, vsg::ref_ptr<vsg::Object>>;
#endif

        std::string _readPropertyName;
        ObjectIDMap _objectIDMap;
        vsg::ref_ptr<vsg::ObjectFactory> _objectFactory;
    };

} // namespace vsg
