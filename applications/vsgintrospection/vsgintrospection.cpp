#include <vsg/introspection/c_interface.h>

int main(int /*argc*/, char** /*argv*/)
{

    vsgObjectPtr group = vsgCreate("vsg::Group");

    vsgRef(group);

    vsgUnref(group);

    return 0;
}
