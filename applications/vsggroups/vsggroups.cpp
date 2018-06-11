#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/nodes/Group.h>

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Group>
#include <osg/Timer>
#include <osg/Observer>
#include <osg/UserDataContainer>

#include <iostream>
#include <vector>
#include <chrono>


class ExplicitVsgVisitor : public vsg::Visitor
{
public:

    ExplicitVsgVisitor():
        numGroups(0)
    {}

    unsigned int numGroups;

    void apply(vsg::Object& object)
    {
        object.traverse(*this);
    }

    void apply(vsg::Group& group)
    {
        ++numGroups;
        group.vsg::Group::traverse(*this);
    }
};

class VsgVisitor : public vsg::Visitor
{
public:

    VsgVisitor():
        numGroups(0)
    {}

    unsigned int numGroups;

    void apply(vsg::Object& object)
    {
        object.traverse(*this);
    }

    void apply(vsg::Group& group)
    {
        ++numGroups;
        group.traverse(*this);
    }
};

class OsgVisitor: public osg::NodeVisitor
{
public:

    OsgVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        numGroups(0)
    {}

    unsigned int numGroups;

    void apply(osg::Group& group)
    {
        ++numGroups;
        traverse(group);
    }
};


template<class T>
T* createQuadTree(unsigned int numLevels)
{
    T* t = new T;
    if (numLevels==0) return t;

    --numLevels;

    t->addChild(createQuadTree<T>(numLevels));
    t->addChild(createQuadTree<T>(numLevels));
    t->addChild(createQuadTree<T>(numLevels));
    t->addChild(createQuadTree<T>(numLevels));

    return t;
}

template<>
vsg::Group* createQuadTree(unsigned int numLevels)
{
    vsg::Group* t = new vsg::Group;
    if (numLevels==0) return t;

    --numLevels;

    t->getChildren().reserve(4);

    t->addChild(createQuadTree<vsg::Group>(numLevels));
    t->addChild(createQuadTree<vsg::Group>(numLevels));
    t->addChild(createQuadTree<vsg::Group>(numLevels));
    t->addChild(createQuadTree<vsg::Group>(numLevels));

    return t;
}

class ElapsedTime
{
public:
    using clock = std::chrono::high_resolution_clock;
    clock::time_point _start;

    ElapsedTime()
    {
        start();
    }

    void start()
    {
        _start = clock::now();
    }

    double duration() const
    {
        return std::chrono::duration<double>(clock::now()-_start).count();
    }
};

int main(int argc, char** argv)
{
    ElapsedTime timer;

    unsigned int numLevels = 10;
    timer.start();
    osg::ref_ptr<osg::Group> osg_group = createQuadTree<osg::Group>(numLevels);
    std::cout<<"OpenSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    osg::ref_ptr<vsg::Group> vsg_group = createQuadTree<vsg::Group>(numLevels);
    std::cout<<"VulkanSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    std::cout<<std::endl;

    timer.start();
    osg_group = nullptr;
    std::cout<<"OpenSceneGraph deletion : "<<timer.duration()<<std::endl;

    timer.start();
    vsg_group = nullptr;
    std::cout<<"VulkanSceneGraph deletion : "<<timer.duration()<<std::endl;

    std::cout<<std::endl;

    timer.start();
    osg_group = createQuadTree<osg::Group>(numLevels);
    std::cout<<"OpenSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    vsg_group = createQuadTree<vsg::Group>(numLevels);
    std::cout<<"VulkanSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    std::cout<<std::endl;



    timer.start();
    VsgVisitor vsgVisitor;
    vsg_group->accept(vsgVisitor);
    double normal = timer.duration();

    timer.start();
    ExplicitVsgVisitor evsgVisitor;
    vsg_group->accept(evsgVisitor);
    double explicit_duration = timer.duration();

    timer.start();
    OsgVisitor osgVisitor;
    osg_group->accept(osgVisitor);
    double osg_duration = timer.duration();


    std::cout<<"OpenScenGraph Quad Tree traverse : "<<osg_duration<<" "<<osgVisitor.numGroups<<std::endl;
    std::cout<<"VulkanSceneGraph Quad Tree traverse normal    : "<<normal<<" "<<vsgVisitor.numGroups<<std::endl;
    std::cout<<"VulkanSceneGraph Quad Tree traverse, explicit : "<<explicit_duration<<" "<<evsgVisitor.numGroups<<std::endl;
    std::cout<<"normal/explicit_duration : "<<normal/explicit_duration<<" "<<std::endl;
    std::cout<<"osg/normal traversal duration : "<<osg_duration/normal<<" "<<std::endl;

    return 0;
}