#include "ref_ptr.hpp"
#include "Object.hpp"
#include "Group.hpp"

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Group>
#include <osg/Timer>

#include <iostream>
#include <vector>
#include <chrono>

vsg::Group* createGroup()
{
    vsg::ref_ptr<vsg::Group> group = new vsg::Group;
    return group.release();
}

class OsgClass : public osg::Referenced
{
public:

    OsgClass()
    {
        std::cout<<"OsgClass::OsgClass() "<<this<<std::endl;
    }

protected:
    virtual ~OsgClass()
    {
        std::cout<<"OsgClass::~OsgClass() "<<this<<std::endl;
    }
};


int main(int argc, char** argv)
{
    auto start = std::chrono::high_resolution_clock::now(); // will eventually need to use chrono::steady_clock for frame stats

    std::cout<<"-- Before"<<std::endl<<std::endl;
    vsg::ref_ptr<vsg::Object> global;
    {
        std::cout<<"---- Start of block"<<std::endl;

//        vsg::ref_ptr<vsg::Group> group(createGroup());
        vsg::ref_ptr<vsg::Group> group(new vsg::Group());

        std::cout<<"Adding child to group"<<std::endl;
        size_t pos = group->addChild(new vsg::Node);

        group->addChild(new vsg::Group);

        std::cout<<"++++ Removing child to group"<<std::endl;
        group->removeChild(pos);


        vsg::ref_ptr<OsgClass> osg = new OsgClass;

        std::cout<<"size_of<OsgClass> "<<sizeof(OsgClass)<<std::endl;
        std::cout<<"size_of<osg::Node> "<<sizeof(osg::Node)<<std::endl;
        std::cout<<"size_of<vsg::Node> "<<sizeof(vsg::Node)<<std::endl;
        std::cout<<"size_of<osg::Grouo> "<<sizeof(osg::Group)<<std::endl;
        std::cout<<"size_of<vsg::Grouo> "<<sizeof(vsg::Group)<<std::endl;

        global = group;
        std::cout<<"---- End of block"<<std::endl<<std::endl;
    }

    global = nullptr;

    auto end = std::chrono::high_resolution_clock::now();
    double std_duration = std::chrono::duration<double>(end-start).count();
    std::cout<<"-- After, elapsed time "<<std_duration<<std::endl;

    return 0;
}