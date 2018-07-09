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

namespace vsg
{
    /** convinience template pointer class for adapting ref_ptr<> or C pointers into a pointer<> to simplify passing either type of pointer to a method/function.
        pointer<> does not incrment or decrement the reference count of the object, and behaves like a C pointer for all intents and purposes.*/
    template<class T>
    class pointer
    {
    public:

        pointer(T* ptr) : _ptr(ptr) {}

        template<class R>
        pointer(ref_ptr<R> ptr) : _ptr(ptr.get()) {}

        pointer& operator = (T* ptr)
        {
            _ptr = ptr;
            return *this;
        }

        template<class R>
        pointer& operator = (const ref_ptr<R>& rhs)
        {
            _ptr = rhs.get();
            return *this;
        }

        operator T* () const { return _ptr; }

        operator ref_ptr<T> () const { return ref_ptr<T>(_ptr); }

        bool valid() const { return _ptr!=nullptr; }

        bool operator!() const { return _ptr==nullptr; }

        T& operator*() const { return *_ptr; }

        T* operator->() const { return _ptr;}

        T* get() const { return _ptr; }

    protected:
        T* _ptr;
    };
}


void addChild(vsg::pointer<vsg::Group> group, vsg::pointer<vsg::Node> child)
{
    std::cout<<"addChild( "<<group.get()<<", "<<child.get()<<")"<<std::endl;
    group->addChild(child);
}

int main(int argc, char** argv)
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


    vsg::ref_ptr<vsg::Group> group = new vsg::Group;
    vsg::Node* child = new vsg::Node;
    addChild(group, child);

    vsg::pointer<vsg::Group> ptr_group = group;
    vsg::ref_ptr<vsg::Group> second_group_ptr = ptr_group;


    return 0;
}