# Disign Principles and Philosophy

> *"Successful Software Lives For Ever"*

This document introduces the design principles/philiosphy of the project lead Robert Osfield.  The principles draw upon the experience of being project lead of the [OpenSceneGraph](https://www.openscenegraph.org) and leassons learned from the real-time compute graphics, C++, and open source/free software communities.

First lets break down the opening phrase, *"Successful Software Lives For Ever"*, one I coined for a training course over a decade ago:

* What is *"Success Software"* : Software that is useful and reliable enough to developers and/or users to adopt it as a part of their work or personal lives and rely upon it to live for as long as they need it.

* How long is *"For Ever"* : Long enough that time during maintainance and evolution is significantly longer than initial time to develop the first stable release.  Software that is unreliable, hard to maintain and evolve becomes unsustainable in the long term, it looses it's relevance because of it and fails. Success dmands good design, implementation and responsive curation over many years.


For the OpenSceneGraph project what made it a success was it achieved ***performance*** levels that competed well with the best propritary and open source scene graphs, and was well designed enough to help with developers be ***productive*** over long term application development and deployment. Responsive of contributors to support of community  has also been crucial for the OpenSceneGraph remaining relevent to real-time graphics application developers for nearly two decades.

Technology moves on, Vulkan is displacing OpenGL is the graphics API of choice for high performance cross platform graphics development.  C++ has begun evolving more rapidly and now C++17 offers many features that make appplications faster, more robust and easier to develop and maintain. Ideas on scene graphs have also advanced, a well designed clean room scene graph has the potential for improving graphics and compute performance and developer productivity beyond the previous start of art.  The VulkanSceneGraph project goal is to embody the potential of Vulkan and modern C++ and create middle-ware for next generation of graphics applications, and for it to remain a valued tool for the next decade and beyond.

There are two broad areas that will determine the ***Success*** of VulkanSceneGraph : ***Performance*** and ***Productivity***.  The rest of this document will break these down areas and discuss the design principles that will aim to deliver in these areas.  I'll coin another phrase to emphasise how both are equally crucial to success:

> ***"You live for Performance, but die a slow, painful death without Productivity"***

## Performance

Performance can be mean different things to different applications, some applications maximum framerate may be the goal, others frame drops when targetting a fixed frame rate is critical, while others may look to minimize power demands required to achieve a target frame rate and visual quality. In all these cases overall efficiency of taking a representation of 2d or 3d world and rendering it on a GPU the key determiner, the lower overhead on the complete computer system the better the effeciency and potential performance.

The stages of work that are crucial to undertake effeciently with a graphics application are:

1. Creation and destructions of scenes, paged scenes must run in a parallel to rendering
1. Upating the scene(s)
1. Cull traversal - view frustum etc. culling to generate a dispatch graph
1. Dispatch traversal - sending the data in the dispatch graph to the GPU(s)
1. Graphics Processing - graohcis and compute work done on the GPU(s)

The adoption of Vulkan provides a significant reduction in CPU overhed with dispatching data compared to OpenGL and Direct3D, this immediately reduce cost of the stage 4 - dispatch traversl. However, the benefits are only fully realized if the scene graph overhead involved in dispatch traversal stage and cost of paging, updating and cull traversal are proportionally reduced.  To deliver on all the potential benefits that Vulkan has the scene graph must take similiar strides forwards in reducing overheads and improving efficiency.

The principles used as guide to achieving effeciency are:

* Minimize memory foorprint of scene graph objects
* Avoid non essential data storage
* Pack commonly accessed data for cache friendly access
* Avoid non essential conditionals
* Choose coding techniques that are friendly compiler to optimization and CPU parallism

An example of minziming footprint, non essential conditions and data storage has already impacted the design and implementation can be seen in minizing the size of the core [vsg::Object](../../include/vsg/core/Object.h) that is backbone of the scene graph by moving all optional data out into an optional [vsg::Auxiliary](../../include/vsg/core/Auxiliary.h) class.  This change reduces the size of vsg::Object to 24 bytes, compared to the OpenSceneGraph's osg::Object class that is 72 bytes.  The vsg::Node class adds no exta data members overhead so remains at 24 bytes, while the OpenSceneGraph's osg::Node class footprint is 20 bytes.  These memory footprint reductions are carried over to all objects in the scene graph.

Traversals of a scene graph and dipatch graphs are the main operations that a scene graph undertakes each frame, cache misses and lower the cycle overhead per object visited is key to reducing traversal times.  The memory footprint reduction immediately reduces the number of page fault that occurm and avoiding non essential conditionals provides a second improvement as it reduces number of cycles required per object visited.

The way that avoiding non essential conditionals has been addressed is to drop the NodeMask and TraversalMode parameters that are found in the OpenSceneGraph's on all osg::Node and osg::NodeVisitor respectively.  When NodeMask functionality is required in a scene graph then task will fall to a MaskGroup that will have a local mask and undertake the conditional during traversals that is required, so that only scene graphs that require a mask will pay the penalty for it.  Decision of what type of traversal to understake is also moved to the Visitor subclass rather than the Visitor base class.  Finally the NodePath that is automatically accumulated by the OpenSceneGraph's NodeVisitor is also dispenced with, if Visitor implementation require this functionality then they are left to implement it.

To test the effectiveness of these design differences a test program, vsgroups (found in vsgFrameworks project) was used.  The results these seemmilingly small design changes over the OpenSceneGraph have a dramatic improvement in performance.

* quad tree construction and destruction times are 3 x faster in VSG vs OSG.
* quad tree traversal is 6 to 10 x faster in VSG vs OSG (depends upon Node and Visitor type.)

The reason for this dramatic improvement is due to:

* singificnat reduction in page faults due to memory footprint reduction
* reduction in conditionals, reducing number of instructions per object visited and improving CPU ability to prefetch and speculativity execute
* improvement in number of instructions per cycle that the CPU can sustain

These are benefits even before we compared Vulkan vs OpneGL improvements, it's still too early in the projects life to be able to compare on realistic scenes, as things progress we'll provide more results.  We cna be confident that the improvements in efficiency of the scene graph traversals combined with the efficiency of Vulkan will substaintially improve the ability to have large and complex worlds, and reduce the power over-head required to achieve a specific level of visual quality.


## Productivity

To be written up.



