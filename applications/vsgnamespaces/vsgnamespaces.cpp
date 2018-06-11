#include <vsg/Object>

namespace vsg
{
    namespace core
    {
        class CoreClass : public vsg::Object
        {
        };
    }

    namespace maths
    {
        class mat4
        {
        };
    }

    namespace viewwer
    {
        class Viewer : public vsg::Object
        {
        };

        class View : public vsg::Object
        {
        };
    }
};


int main(int argc, char** argv)
{
    return 0;
};