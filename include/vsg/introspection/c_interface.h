#ifndef VSG_C_INTERFACE
#define VSG_C_INTERFACE

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
            vsgObjectPtr    _object;
            char            _bool;
            char            _char;
            unsigned char   _unsigned_char;
            short           _short;
            unsigned short  _unsigned_short;
            int             _int;
            unsigned int    _unsigned_int;
            float           _float;
            double          _double;
            float           _vec2[2];
            float           _vec3[3];
            float           _vec4[4];
            double          _dvec2[2];
            double          _dvec3[3];
            double          _dvec4[4];
            float           _mat4[4][4];
            double          _dma4t[4][4];
        } value;
    };

    extern VSG_EXPORT void vsgRef(vsgObjectPtr object);

    extern VSG_EXPORT void vsgUnref(vsgObjectPtr object);

    extern VSG_EXPORT vsgObjectPtr vsgCreate(const char* className);

    extern VSG_EXPORT const char* vsgClassName(vsgObjectPtr object);

    extern VSG_EXPORT vsgObjectPtr vsgMethod(vsgObjectPtr object, const char* methodName);

    extern VSG_EXPORT vsgObjectPtr vsgGetProperty(vsgObjectPtr object, const char* propertyName);

    extern VSG_EXPORT vsgObjectPtr vsgSetProperty(vsgObjectPtr object, const char* propertyName, vsgObjectPtr* value);

    extern VSG_EXPORT unsigned int vsgGetNumProperties(vsgObjectPtr object);

    extern VSG_EXPORT const char* vsgGetPropertyName(vsgObjectPtr object, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif
