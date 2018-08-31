#include <vsg/introspection/c_interface.h>

#include <stdio.h>

int main(int argc, char** argv)
{

    struct Property bool_property;
    bool_property.type = TYPE_bool;
    bool_property.value._bool = 1;

    struct Property mat4_property;
    mat4_property.type = TYPE_mat4;
    mat4_property.value._mat4[0][0] = 1.0f;

    vsgObjectPtr group = vsgCreate("vsg::Group");
    vsgRef(group);

    vsgObjectPtr lod = vsgCreate("vsg::LOD");
    vsgRef(lod);

    vsgObjectPtr node = vsgCreate("vsg::Node");
    vsgRef(node);

    vsgObjectPtr draw = vsgCreate("vsg::Draw");
    vsgRef(draw);

    struct Property object_property;
    object_property.type = TYPE_Object;
    object_property.value._object = group;

    const char* groupClassName = vsgClassName(group);
    if (groupClassName) printf("group's ClassName = %s\n", groupClassName);
    else printf("group's no ClassNae found.\n");

    printf("object_property.type = %i \n", object_property.type);
    printf("object_property.value = %i \n", object_property.value._int);

    struct Property copy_property = object_property;

    printf("copy_property.type = %i \n", copy_property.type);
    printf("copy_property.value = %i \n", copy_property.value._int);


    vsgUnref(draw);
    vsgUnref(node);
    vsgUnref(lod);
    vsgUnref(group);

    return 0;
}
