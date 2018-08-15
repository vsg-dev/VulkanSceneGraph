#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Value.h>

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

vsg::Group* createGroup()
{
    vsg::ref_ptr<vsg::Group> group = new vsg::Group;
    return group.release();
}

class MyVisitor : public vsg::Visitor
{
public:
    MyVisitor() {}

    void apply(vsg::Object& object)
    {
        std::cout<<"Myvisitor::apply(Object& "<<&object<<")"<<std::endl;
    }

    void apply(vsg::Node& node)
    {
        std::cout<<"Myvisitor::apply(Node& "<<&node<<")"<<std::endl;
    }

    void apply(vsg::Group& group)
    {
        std::cout<<"Myvisitor::apply(Group& "<<&group<<")"<<std::endl;
        for (size_t i=0; i<group.getNumChildren(); ++i)
        {
            group.getChild(i)->accept(*this);
        }
    }
};

int main(int /*argc*/, char** /*argv*/)
{

    auto start = std::chrono::high_resolution_clock::now(); // will eventually need to use chrono::steady_clock for frame stats
    std::cout<<"-- Before"<<std::endl<<std::endl;
    vsg::ref_ptr<vsg::Group> global;
    {
        std::cout<<"---- Start of block"<<std::endl;

//        vsg::ref_ptr<vsg::Group> group(createGroup());
        vsg::ref_ptr<vsg::Group> group(new vsg::Group());

        std::cout<<"Adding child to group"<<std::endl;
        group->addChild(new vsg::Node);
        group->addChild(new vsg::Group);

        group->addChild(new vsg::Group);

        //std::cout<<"++++ Removing child to group"<<std::endl;
        //group->removeChild(pos);

        group->setObject("userdata", new vsg::Object());
        group->setObject(10, new vsg::Object());
        group->setObject(vsg::Object::Key("list",5), new vsg::Object());

        group->setValue("name", std::string("[first node]"));
        group->setValue("height", 1.52f);


        //vsg::ref_ptr<OsgClass> osg = new OsgClass;


        global = group;
        std::cout<<"---- End of block"<<std::endl<<std::endl;
    }

    MyVisitor visitor;
    global->accept(visitor);


    std::cout<<" global->getObject(\"userdata\") = "<<global->getObject("userdata")<<std::endl;

    std::string name;
    if (global->getValue("name", name)) std::cout<<" global->getValue(\"name\") = "<<name<<std::endl;
    else std::cout<<" Failed global->getValue(\"name\") = "<<std::endl;

    float height;
    if (global->getValue("height", height)) std::cout<<" global->getValue(\"height\") = "<<height<<std::endl;
    else std::cout<<" Failed global->getValue(\"height\") = "<<std::endl;

    vsg::observer_ptr<vsg::Group> observer;

    //observer = global.get();
    observer = global;

    std::cout<<"******"<<std::endl;
    {
        vsg::ref_ptr<vsg::Group> access = observer;
        std::cout<<"++++ access.get() "<<access.get()<<" "<<access->referenceCount()<<std::endl;
        global = nullptr;
        std::cout<<"++++ after global reset access.get() "<<access.get()<<" "<<access->referenceCount()<<std::endl;
        access = 0;
        access = observer;
        std::cout<<"++++ after observer reassigned to access.get() "<<access.get()<<std::endl;
    }
    std::cout<<"******"<<std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    double std_duration = std::chrono::duration<double>(end-start).count();
    std::cout<<"-- After, elapsed time "<<std_duration<<std::endl;

    std::cout<<std::endl;
    std::cout<<"size_of<osg::Referenced> "<<sizeof(osg::Referenced)<<std::endl;
    std::cout<<"size_of<osg::Object> "<<sizeof(osg::Object)<<std::endl;
    std::cout<<"size_of<osg::Node> "<<sizeof(osg::Node)<<std::endl;
    std::cout<<"size_of<osg::Grouo> "<<sizeof(osg::Group)<<std::endl;
    std::cout<<"size_of<osg::ObserverSet> "<<sizeof(osg::ObserverSet)<<std::endl;
    std::cout<<"size_of<osg::UserDataContainer> "<<sizeof(osg::UserDataContainer)<<std::endl;
    std::cout<<std::endl;
    std::cout<<"size_of<vsg::Object> "<<sizeof(vsg::Object)<<std::endl;
    std::cout<<"size_of<vsg::Node> "<<sizeof(vsg::Node)<<std::endl;
    std::cout<<"size_of<vsg::Grouo> "<<sizeof(vsg::Group)<<std::endl;
    std::cout<<"size_of<vsg::Auxiliary> "<<sizeof(vsg::Auxiliary)<<std::endl;

    return 0;
}
