
#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Array3D.h>
#include <vsg/core/Value.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Visitor.h>

int main()
{
    vsg::ref_ptr<vsg::Array<uint8_t>> a = vsg::ubyteArray::create(1024);
    vsg::ref_ptr<vsg::Array2D<uint8_t>> a2d = vsg::ubyteArray2D::create(1, 2);
    vsg::ref_ptr<vsg::Array3D<uint8_t>> a3d = vsg::ubyteArray3D::create(1, 2, 3);
    vsg::ref_ptr<vsg::Value<unsigned int>> v = vsg::uintValue::create(0);

    return 0;
}
