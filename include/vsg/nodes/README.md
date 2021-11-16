# include/vsg/nodes headers
The **include/vsg/nodes** header directory contains the graph node classes.  These classes are used to create scene graphs, command graphs and graphs used for custom purposes.

The design is based on the concept of top down Direct Acyclic Graph (DAG), where the internal nodes of the graph aggregate lists of smart pointers to other internal or leaf nodes. Node subclasses add specific data and behaviours to the graph to provided guidance how the graph should be traversed and processed.

## Node base class
* [include/vsg/nodes/Node.h](Node.h) - base Node class, currently it doesn't provide any functionality and just serves as a base class, it's role will likely expand as the project advances.
* [include/vsg/nodes/Group.h](Group.h) - general purpose group that aggregates a list of children using variable sized vector of `vsg::ref_ptr<vsg::Node>`.
* [include/vsg/nodes/QuadGroup.h](QuadGroup.h) - a performance orientated group with sized fixed to four children.
* [include/vsg/nodes/LOD.h](LOD.h) - level of detail node with two children and one distance value to guidance choice between them.
* [include/vsg/nodes/PagedLOD.h](PagedLOD.h) - level of detail node with two children and one distance value to guidance choice between them, with one child as externally paged file.
* [include/vsg/nodes/Switch.h](Switch.h) - Switch node for toggling on/off the recording of individual children.
* [include/vsg/nodes/StateGroup.h](StateGroup.h)

