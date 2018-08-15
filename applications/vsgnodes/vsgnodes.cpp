#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/LOD.h>

#include <iostream>
#include <vector>
#include <chrono>

struct PrintVisitor : public vsg::Visitor
{
    void apply(vsg::Object& object)
    {
        std::cout<<"apply(vsg::Object& "<<&object<<")"<<std::endl;
        object.traverse(*this);
    }

    void apply(vsg::Node& node)
    {
        std::cout<<"apply(vsg::Node& "<<&node<<")"<<std::endl;
        node.traverse(*this);
    }

    void apply(vsg::Group& group)
    {
        std::cout<<"apply(vsg::Group& "<<&group<<") getNumChildren()="<<group.getNumChildren()<<std::endl;
        group.traverse(*this);
    }

    void apply(vsg::QuadGroup& group)
    {
        std::cout<<"apply(vsg::QuadGroup& "<<&group<<") getNumChildren()="<<group.getNumChildren()<<std::endl;
        group.traverse(*this);
    }

    void apply(vsg::LOD& lod)
    {
        std::cout<<"apply(vsg::LOD& "<<&lod<<") getNumChildren()="<<lod.getNumChildren()<<std::endl;
        lod.traverse(*this);
    }
};

int main(int /*argc*/, char** /*argv*/)
{

    vsg::ref_ptr<vsg::Group> group = new vsg::Group;
    //vsg::ref_ptr<vsg::QuadGroup> quad = new vsg::Quad;

    // set up LOD
    vsg::ref_ptr<vsg::LOD> lod = new vsg::LOD;
    lod->setMinimumArea(0, 0.0);
    lod->setChild(0, new vsg::Node);
    lod->setMinimumArea(1, 0.0);
    lod->setChild(1, new vsg::Node);
    group->addChild(lod.get());

    PrintVisitor visitor;
    group->accept(visitor);


    return 0;
}
