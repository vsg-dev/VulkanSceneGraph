#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/FixedGroup.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <functional>


vsg::Node* createQuadTree(unsigned int numLevels)
{
    if (numLevels==0) return new vsg::Node;

    vsg::Group* t = new vsg::Group;

    --numLevels;

    t->getChildren().reserve(4);

    t->addChild(createQuadTree(numLevels));
    t->addChild(createQuadTree(numLevels));
    t->addChild(createQuadTree(numLevels));
    t->addChild(createQuadTree(numLevels));

    return t;
}


template<typename F>
double time(F function)
{
    using clock = std::chrono::high_resolution_clock;
    clock::time_point start = clock::now();

    // do test code
    function();

    return std::chrono::duration<double>(clock::now()-start).count();
}


template<typename F1, typename C1>
class LambdaVisitor : public vsg::Visitor
{
public:

    LambdaVisitor(F1 func) : _function1(func) {}

    F1 _function1;

    void apply(C1& object)
    {
        _function1(object);
        object.traverse(*this);
    }
};

template<typename P, typename F, typename C=vsg::Object>
void visit(P& object, F function)
{
    LambdaVisitor<F,C> lv(function);
    object->accept(lv);
}

template<typename F1, typename C1, typename F2, typename C2>
class Lambda2Visitor : public vsg::Visitor
{
public:

    Lambda2Visitor(F1 func1, F2 func2) : _function1(func1), _function2(func2) {}

    F1 _function1;
    F2 _function2;

    void apply(C1& object)
    {
        _function1(object);
        object.traverse(*this);
    }

    void apply(C2& object)
    {
        _function2(object);
        object.traverse(*this);
    }
};

class FunctionVisitor : public vsg::Visitor
{
public:

    std::function<void(vsg::Object&)>   objectFunction;
    std::function<void(vsg::Node&)>     nodeFunction;
    std::function<void(vsg::Group&)>    groupFunction;

    void apply(vsg::Object& object)
    {
        if (objectFunction) objectFunction(object);
        object.traverse(*this);
    }

    void apply(vsg::Node& node)
    {
        if (nodeFunction) { nodeFunction(node); node.traverse(*this); }
        else apply(static_cast<vsg::Object&>(node));
    }

    void apply(vsg::Group& group)
    {
        if (groupFunction) { groupFunction(group); group.traverse(*this); }
        else apply(static_cast<vsg::Node&>(group));
    }
};

template<typename P, typename F1, typename C1, typename F2, typename C2>
void visit(P& object, F1 function1, F2 function2)
{
    Lambda2Visitor<F1, C1, F2, C2> lv(function1, function2);
    object->accept(lv);
}

template<typename P, typename C1>
void visit2(P& object, std::function<void(C1&)> func1)
{
    LambdaVisitor<decltype(func1), C1> lv(func1);
    object->accept(lv);
}

template<typename P, typename C1, typename C2>
void visit2(P& object, std::function<void(C1&)> func1, std::function<void(C2&)> func2)
{
    Lambda2Visitor<decltype(func1), C1, decltype(func2), C2> lv(func1, func2);
    object->accept(lv);
}


int main(int argc, char** argv)
{
    unsigned int numLevels = 11;

    vsg::ref_ptr<vsg::Node> scene;
    std::cout<<"VulkanSceneGraph Fixed Quad Tree creation : "<<time( [&]() { scene = createQuadTree(numLevels); } )<<std::endl;


    unsigned int count=0;
    auto countFunc = [&](vsg::Object& object) { ++count; };
    LambdaVisitor<decltype(countFunc), vsg::Object> lv( countFunc ); // note, using of decltype(auto) is a C++14 feature
    scene->accept(lv);
    std::cout<<"LambdaVisitor "<<count<<std::endl;


    count = 0;
    visit(scene, [&](vsg::Object& object) { ++count; } );
    std::cout<<"visit() count="<<count<<std::endl;


    int nodes = 0, groups = 0;
    auto countFunc1 = [&](vsg::Node&) { ++nodes; };
    auto countFunc2 = [&](vsg::Group&) { ++groups; };

    visit<vsg::ref_ptr<vsg::Node>, decltype(countFunc1), vsg::Node, decltype(countFunc2), vsg::Group>(scene, countFunc1, countFunc2);
    std::cout<<"visit() nodes="<<nodes<<", groups="<<groups<<std::endl;


    struct MyVisitor : public vsg::Visitor
    {
        unsigned int objects = 0;
        unsigned int groups = 0;

        void apply(vsg::Object& object)
        {
            ++objects;
            object.traverse(*this);
        }

        void apply(vsg::Group& group)
        {
            ++groups;
            group.traverse(*this);
        }
    };

    MyVisitor myVisitor;
    scene->accept(myVisitor);
    std::cout<<"MyVisitor() objects="<<myVisitor.objects<<", groups="<<myVisitor.groups<<std::endl;


    count = 0;
    visit2<vsg::ref_ptr<vsg::Node>, vsg::Object>(scene, [&](vsg::Object&) { ++count; } );

    std::cout<<"visit2 count="<<count<<std::endl;

    nodes = 0; groups = 0;
    visit2<vsg::ref_ptr<vsg::Node>, vsg::Object, vsg::Group>(
        scene,
        [&](vsg::Object&) { ++nodes; },
        [&](vsg::Group&) { ++groups; });

    std::cout<<"visit2 nodes="<<nodes<<", groups="<<groups<<std::endl;

    int objects = 0;
    nodes = 0;
    groups = 0;

    FunctionVisitor fv;
    fv.objectFunction = [&](vsg::Object&) { ++objects; };
    fv.nodeFunction = [&](vsg::Node&) { ++nodes; };
    fv.groupFunction = [&](vsg::Group& group) { ++groups; };

    scene->accept(fv);

    std::cout<<"FunctionVisitor objects="<<objects<<", nodes="<<nodes<<", groups="<<groups<<std::endl;

    return 0;
}