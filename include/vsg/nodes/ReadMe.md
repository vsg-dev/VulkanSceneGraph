# include/vsg/nodes headers
The **include/vsg/nodes** header directory contains the graph node classes.  These class are used to create scene graphs, command graphs and graphs used for custom purposes.  

The design is based on the concept of top down Direct Acyclic Graph (DAG), where the internal nodes of the graph aggregate lists of smart pointers to other internal or leaf nodes. Node subclasses add specific data and behaviours to the graph to provided guidance how the graph should be traversed and processed.

## Node base class
[include/vsg/nodes/nodes/Node.h](Node.h) - base Node class, currently it doesn't provide any functionality and just serves as a base class, it's role will likely expand as the project advances.

## Group classes
[include/vsg/nodes/nodes/Group.h](Group.h) - general purpose group that aggregates a list of children using variable sized vector of `vsg::ref_ptr<vsg::Node>`.

[include/vsg/nodes/nodes/QuadGroup.h](QuadGroup.h) - a performance orientated group with sized fixed to four children.

[include/vsg/nodes/FixedGroup.h](FixedGroup.h) - a performance orientated template class for agragating a fixed sized group, with the size determined at compile time.

## LOD class
[include/vsg/nodes/nodes/LOD.h](LOD.h) - an experiment with a stripped down level of details class that has just two children and one distance value to guidance choice between them.

## State classes
[include/vsg/nodes/nodes/StateGroup.h](StateGroup.h) - a subclass from vsg::Group that add a list of `ref_ptr<vsg::StateComponent>`that encapsulate Vulkan state such as shader, uniform and vertex bindings.