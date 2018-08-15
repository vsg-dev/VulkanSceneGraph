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
#include <memory>


class SharerdPtrAuxilary : public std::enable_shared_from_this<SharerdPtrAuxilary>
{
public:
    SharerdPtrAuxilary() {}

protected:
    virtual ~SharerdPtrAuxilary() {}
};

class SharedPtrNode : public std::enable_shared_from_this<SharedPtrNode>
{
public:
    SharedPtrNode() {}

protected:
    virtual ~SharedPtrNode() {}

    std::shared_ptr<SharerdPtrAuxilary> _auxilary;
};

class SharedPtrQuadGroup : public SharedPtrNode
{
public:

    SharedPtrQuadGroup() {}

    using Children = std::array<std::shared_ptr<SharedPtrNode>, 4>;

    void setChild(std::size_t i, SharedPtrNode* node) { _children[i] = node ? node->shared_from_this() : nullptr; }
    SharedPtrNode* getChild(std::size_t i) { return _children[i].get(); }
    const SharedPtrNode* getChild(std::size_t i) const { return _children[i].get(); }

//protected:
    virtual ~SharedPtrQuadGroup() {}

    Children _children;

};

int main(int /*argc*/, char** /*argv*/)
{

    vsg::ref_ptr<vsg::QuadGroup> ref_node = new vsg::QuadGroup;
    std::cout<<"size_of(vsg::ref_ptr)="<<sizeof(ref_node)<<", sizeof(QuadGroup)="<<sizeof(vsg::QuadGroup)<<std::endl;


#if 1
    std::shared_ptr<SharedPtrQuadGroup> shared_node(new SharedPtrQuadGroup);
#else
    // does not compile : error: conversion from ‘std::shared_ptr<SharedPtrNode>’ to non-scalar type ‘std::shared_ptr<SharedPtrQuadGroup>’
    std::shared_ptr<SharedPtrQuadGroup> shared_node = (new SharedPtrQuadGroup)->shared_from_this();  requested
#endif

    std::cout<<"size_of(std::shared_ptr)="<<sizeof(shared_node)<<", sizeof(SharedPtrQuadGroup)="<<sizeof(SharedPtrQuadGroup)<<std::endl;

    return 0;
}
