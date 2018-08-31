#include <vsg/introspection/c_interface.h>

#include <stdio.h>

int main(int argc, char** argv)
{

    vsgObjectPtr group = vsgCreate("vsg::Group");
    vsgRef(group);

    vsgObjectPtr lod = vsgCreate("vsg::LOD");
    vsgRef(lod);

    vsgObjectPtr node = vsgCreate("vsg::Node");
    vsgRef(node);

    vsgObjectPtr draw = vsgCreate("vsg::Draw");
    vsgRef(draw);

    const char* groupClassName = vsgClassName(group);
    if (groupClassName) printf("group's ClassName = %s\n", groupClassName);
    else printf("group's no ClassNae found.\n");


    vsgUnref(draw);
    vsgUnref(node);
    vsgUnref(lod);
    vsgUnref(group);

    return 0;
}
