#ifndef VSG_C_INTERFACE
#define VSG_C_INTERFACE

#include <vsg/core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void* vsgObjectPtr;

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
