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


namespace vsg
{
    template<int N>
    class FixedGroup : public vsg::Node
    {
    public:
        FixedGroup() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            //std::cout<<"FixedGroup::traverse(visitor) "<<this<<" getNumChildren()="<<getNumChildren()<<std::endl;
#if 0
            std::for_each(_children.begin(), _children.end(), [&visitor](ref_ptr<Node>& child)
            {
                if (child.valid()) child->accept(visitor);
            });
#else
            for(size_t i=0; i<N; ++i)
            {
                _children[i]->accept(visitor);
            }
#endif
        }

        void setChild(std::size_t pos, vsg::Node* node)
        {
            _children[pos] = node;
            //std::cout<<"FixedGroup::setChild("<<pos<<", "<<node<<")"<<std::endl;
        }

        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return N; }

        using Children = std::array< ref_ptr< vsg::Node>, N >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        virtual ~FixedGroup() {}

        Children _children;
    };

#if 0
    class QuadGroup : public vsg::Node
    {
    public:
        QuadGroup() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            //std::cout<<"FixedGroup::traverse(visitor) "<<this<<" getNumChildren()="<<getNumChildren()<<std::endl;
#if 1
            std::for_each(_children.begin(), _children.end(), [&visitor](ref_ptr<Node>& child)
            {
                if (child.valid()) child->accept(visitor);
            });
#else
            for(size_t i=0; i<N; ++i)
            {
                _children[i]->accept(visitor);
            }
#endif
        }

        void setChild(std::size_t pos, vsg::Node* node)
        {
            _children[pos] = node;
            //std::cout<<"FixedGroup::setChild("<<pos<<", "<<node<<")"<<std::endl;
        }

        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return 4; }

        using Children = std::array< ref_ptr< vsg::Node>, 4 >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        inline virtual ~QuadGroup() {}

        Children _children;
    };
#else
    using QuadGroup = FixedGroup<4>;
#endif
}



class ExplicitVsgVisitor : public vsg::Visitor
{
public:

    ExplicitVsgVisitor():
        numNodes(0)
    {}

    unsigned int numNodes;

    void apply(vsg::Object& object)
    {
        ++numNodes;
        object.traverse(*this);
    }

    void apply(vsg::Group& group)
    {
        ++numNodes;
        group.vsg::Group::traverse(*this);
    }
};

class VsgVisitor : public vsg::Visitor
{
public:

    VsgVisitor():
        numNodes(0)
    {}

    unsigned int numNodes;

    void apply(vsg::Object& object)
    {
        ++numNodes;
        object.traverse(*this);
    }

    void apply(vsg::Group& group)
    {
        ++numNodes;
        group.traverse(*this);
    }
};

class OsgVisitor: public osg::NodeVisitor
{
public:

    OsgVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        numNodes(0)
    {}

    unsigned int numNodes;

    void apply(osg::Node& node)
    {
        ++numNodes;
        node.traverse(*this);
    }

    void apply(osg::Group& group)
    {
        ++numNodes;
        traverse(group);
    }
};


osg::Node* createOsgQuadTree(unsigned int numLevels)
{
    if (numLevels==0) return new osg::Node;

    osg::Group* t = new osg::Group;

    --numLevels;

    t->addChild(createOsgQuadTree(numLevels));
    t->addChild(createOsgQuadTree(numLevels));
    t->addChild(createOsgQuadTree(numLevels));
    t->addChild(createOsgQuadTree(numLevels));

    return t;
}

vsg::Node* createVsgQuadTree(unsigned int numLevels)
{
    if (numLevels==0) return new vsg::Node;

    vsg::Group* t = new vsg::Group;

    --numLevels;

    t->getChildren().reserve(4);

    t->addChild(createVsgQuadTree(numLevels));
    t->addChild(createVsgQuadTree(numLevels));
    t->addChild(createVsgQuadTree(numLevels));
    t->addChild(createVsgQuadTree(numLevels));

    return t;
}


vsg::Node* createFixedQuadTree(unsigned int numLevels)
{
    if (numLevels==0) return new vsg::Node;

    vsg::QuadGroup* t = new vsg::QuadGroup();

    --numLevels;

    t->setChild(0, createFixedQuadTree(numLevels));
    t->setChild(1, createFixedQuadTree(numLevels));
    t->setChild(2, createFixedQuadTree(numLevels));
    t->setChild(3, createFixedQuadTree(numLevels));

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

    unsigned int numLevels = 11;

    timer.start();
    vsg::ref_ptr<vsg::Node> vsg_group = createVsgQuadTree(numLevels);
    std::cout<<"VulkanSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    vsg::ref_ptr<vsg::Node> fg = createFixedQuadTree(numLevels);
    std::cout<<"VulkanSceneGraph Fixed Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    osg::ref_ptr<osg::Node> osg_group = createOsgQuadTree(numLevels);
    std::cout<<"OpenSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

#if 0
    std::cout<<std::endl;

    timer.start();
    osg_group = nullptr;
    std::cout<<"OpenSceneGraph deletion : "<<timer.duration()<<std::endl;

    timer.start();
    vsg_group = nullptr;
    std::cout<<"VulkanSceneGraph deletion : "<<timer.duration()<<std::endl;

    timer.start();
    fg = nullptr;
    std::cout<<"VulkanSceneGraph FixedGroup deletion : "<<timer.duration()<<std::endl;

    std::cout<<std::endl;

    timer.start();
    osg_group = createOsgQuadTree(numLevels);
    std::cout<<"OpenSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    vsg_group = createVsgQuadTree(numLevels);
    std::cout<<"VulkanSceneGraph Quad Tree creation : "<<timer.duration()<<std::endl;

    timer.start();
    fg = createFixedQuadTree(numLevels);
    std::cout<<"VulkanSceneGraph Fixed Quad Tree creation : "<<timer.duration()<<std::endl;
#endif

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

    timer.start();
    VsgVisitor fgVisitor;
    fg->accept(fgVisitor);
    double fg_duration = timer.duration();

    std::cout<<"OpenScenGraph Quad Tree traverse : "<<osg_duration<<" "<<osgVisitor.numNodes<<std::endl;
    std::cout<<"VulkanSceneGraph Quad Tree traverse normal    : "<<normal<<" "<<vsgVisitor.numNodes<<std::endl;
    std::cout<<"VulkanSceneGraph Quad Tree traverse, explicit : "<<explicit_duration<<" "<<evsgVisitor.numNodes<<std::endl;
    std::cout<<"VulkanSceneGraph Fixed Quad Tree traverse, : "<<fg_duration<<" "<<fgVisitor.numNodes<<std::endl;

    std::cout<<std::endl;
    std::cout<<"normal/explicit_duration : "<<normal/explicit_duration<<" "<<std::endl;
    std::cout<<"osg/normal traversal duration : "<<osg_duration/normal<<" "<<std::endl;
    std::cout<<"osg/fg traversal duration : "<<osg_duration/fg_duration<<" "<<std::endl;
    std::cout<<"normal/fg traversal duration : "<<normal/fg_duration<<" "<<std::endl;

    std::cout<<std::endl;
    std::cout<<"size_of<osg::Group> "<<sizeof(osg::Group)<<" with data "<<(sizeof(osg::Group)+sizeof(vsg::ref_ptr<vsg::Node>)*4)<<std::endl;
    std::cout<<"size_of<vsg::Node> "<<sizeof(vsg::Node)<<std::endl;
    std::cout<<"size_of<vsg::Grouo> "<<sizeof(vsg::Group)<<" with data "<<(sizeof(vsg::Group)+sizeof(vsg::ref_ptr<vsg::Node>)*4)<<std::endl;
    std::cout<<"size_of<vsg::QuadGroup> "<<sizeof(vsg::QuadGroup)<<std::endl;

    return 0;
}