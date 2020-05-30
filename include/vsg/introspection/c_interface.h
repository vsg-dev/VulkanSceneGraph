#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void* vsgObjectPtr;

    struct Property
    {
        enum PropertyType
        {
            TYPE_undefined,
            TYPE_Object,
            TYPE_bool,
            TYPE_char,
            TYPE_unsigned_char,
            TYPE_short,
            TYPE_unsigned_short,
            TYPE_int,
            TYPE_unsigned_int,
            TYPE_float,
            TYPE_double,
            TYPE_vec2,
            TYPE_vec3,
            TYPE_vec4,
            TYPE_dvec2,
            TYPE_dvec3,
            TYPE_dvec4,
            TYPE_mat4,
            TYPE_dmat4
        } type;

        union PropertyValue
        {
            vsgObjectPtr _object;
            char _bool;
            char _char;
            unsigned char _unsigned_char;
            short _short;
            unsigned short _unsigned_short;
            int _int;
            unsigned int _unsigned_int;
            float _float;
            double _double;
            float _vec2[2];
            float _vec3[3];
            float _vec4[4];
            double _dvec2[2];
            double _dvec3[3];
            double _dvec4[4];
            float _mat4[4][4];
            double _dma4t[4][4];
        } value;
    };

    extern VSG_DECLSPEC void vsgRef(vsgObjectPtr object);

    extern VSG_DECLSPEC void vsgUnref(vsgObjectPtr object);

    extern VSG_DECLSPEC vsgObjectPtr vsgCreate(const char* className);

    extern VSG_DECLSPEC const char* vsgClassName(vsgObjectPtr object);

    extern VSG_DECLSPEC vsgObjectPtr vsgMethod(vsgObjectPtr object, const char* methodName);

    extern VSG_DECLSPEC struct Property vsgGetProperty(vsgObjectPtr object, const char* propertyName);

    extern VSG_DECLSPEC void vsgSetProperty(vsgObjectPtr object, const char* propertyName, struct Property property);

    extern VSG_DECLSPEC unsigned int vsgGetNumProperties(vsgObjectPtr object);

    extern VSG_DECLSPEC const char* vsgGetPropertyName(vsgObjectPtr object, unsigned int index);

#ifdef __cplusplus
}
#endif
