# Design Principles and Philosophy

> *"Successful Software Lives For Ever"*

This document introduces the design principles/philosophy of the project lead Robert Osfield.  The principles draw upon the experience of being project lead of the [OpenSceneGraph](https://www.openscenegraph.org) and lessons learned from the real-time compute graphics, C++, and open source/free software communities.

First lets break down the opening phrase, *"Successful Software Lives For Ever"*, one I coined for a training course over a decade ago:

* What is *"Successful Software"* : Software that is useful and reliable enough for developers and/or users to adopt it as a part of their work or personal lives and rely upon it to live for as long as they need it.

* How long is *"For Ever"* : Long enough that time during maintenance and evolution is significantly longer than initial time to develop the first stable release. Software that is unreliable, hard to maintain and evolve becomes unsustainable in the long term, it loses it's relevance because of it and fails. Success demands good design, implementation and responsive curation over many years.


For the OpenSceneGraph project what made it a success was it achieved *performance* levels that competed well with the best proprietary and open source scene graphs, and was well designed enough to help developers be *Productive* over long term application development and deployment. Responsive of contributors to support the community has also been crucial for the OpenSceneGraph remaining relevant to real-time graphics application developers for nearly two decades.

Technology moves on, Vulkan is displacing OpenGL as the graphics API of choice for high performance cross platform graphics development.  C++ has begun evolving more rapidly and now C++17 offers many features that make applications faster, more robust and easier to develop and maintain. Ideas on scene graphs have also advanced, a well designed clean room scene graph has the potential for improving graphics and compute performance and developer productivity beyond the previous start of art.  The VulkanSceneGraph project goal is to embody the potential of Vulkan and modern C++ and create middle-ware for the next generation of graphics applications, and for it to remain a valued tool for the next decade and beyond.

There are two broad areas that will determine the success of the VulkanSceneGraph : [***Performance***](#performance) and [***Productivity***](#productivity).  The rest of this document will break these areas down and discuss the design principles that will aim to deliver in these areas. I'll coin another phrase to emphasise how both are equally crucial to success:

> ***"You live for Performance, but die a slow, painful death without Productivity"***

### Making allowances for living at the *Bleeding Edge*

The below discussion outlines what is guiding the path forward - it's our compass heading, as of the fall of 2018, it's still very early days so the code is in flux and is not ready for wide use out in the industry. During this phase of rapid development early adopters will have to accept lower Performance and Productivity associated with working with alpha software.

Early adopters are hugely important for the success of the project, the extra work you have to put in will benefit us all in helping shape and refine the software so that it's fit for purpose by the time we reach our first stable release and genuinely deliver on what we are striving for.


## Performance

Performance can be mean different things to different applications, some applications maximum frame-rate may be the goal, others avoiding frame drops when targeting a fixed frame rate is critical, for VR minimizing latency is crucial, while others may look to minimize power demands required to achieve a target frame rate and visual quality. In all these cases overall efficiency of taking a representation of a 2D or 3D world and rendering it on a GPU is the key determiner, the lower overhead on the complete computer system the better the efficiency and potential performance.

The stages of work that are crucial to undertake efficiently with a graphics application are:

1. Creation and destructions of scenes, paged scenes must run in a parallel to rendering
1. Updating the scene(s)
1. Cull traversal - view frustum etc. culling to generate a dispatch graph
1. Dispatch traversal - sending the data in the dispatch graph to the GPU(s)
1. Graphics Processing - graphics and compute work done on the GPU(s)

The adoption of Vulkan provides a significant reduction in CPU overhead with dispatching data compared to OpenGL and Direct3D(prior to 11), this immediately reduces the cost of stage 4 - dispatch traversal. However, the benefits are only fully realized if the scene graph overhead involved in dispatch traversal stage and cost of paging, updating and cull traversal are proportionally reduced. To deliver on all the potential benefits that Vulkan has the scene graph must take similar strides forwards in reducing overheads and improving efficiency.

The principles used as a guide to achieving efficiency include:

* Benchmarking is fundamental to determining performance bottlenecks and qualifying that changes are effective - beware of premature optimization!
* Minimize memory footprint of scene graph objects
* Avoid non essential data storage
* Pack commonly accessed data for cache friendly access
* Avoid non essential conditionals
* Choose coding techniques that are friendly to compiler optimization and CPU parallelism

An example of minimizing footprint, non essential conditions and data storage has already impacted the design and implementation can be seen in minimizing the size of the core [vsg::Object](../../include/vsg/core/Object.h) that is backbone of the scene graph by moving all optional data out into an optional [vsg::Auxiliary](../../include/vsg/core/Auxiliary.h) class.  This change reduces the size of vsg::Object to 24 bytes, compared to the OpenSceneGraph's osg::Object class that is 72 bytes.  The vsg::Node class adds no extra data members overhead so remains at 24 bytes, while the OpenSceneGraph's osg::Node class footprint is 20 bytes.  These memory footprint reductions are carried over to all objects in the scene graph.

Scene graphs are fundamentally a graph of objects connected by pointers between those objects.  The size of those objects is inextricably connected to the size of pointers. C++11 onwards provides the std::shared_ptr<> which on 64-bit systems has a size of 16 bytes, while the VSG's intrusive reference counting enables the vsg::ref_ptr<> to have a size of 8 bytes.  In experiments with creation of a quad tree scene graph using std::shared_ptr<> vs vsg::ref_ptr<>, the shared_ptr<> results in a 75% more memory used overall, and 65% slower traversal speeds. This significant difference illustrates that one should not assume that C++ core features are always the most efficient tool.

Traversals of a scene graph and dispatch graphs are the main operations that a scene graph undertakes each frame, cache misses and lowering the cycle overhead per object visited is key to reducing traversal times.  The memory footprint reduction immediately reduces the number of page faults that occur and avoiding non essential conditionals provides a second improvement as it reduces number of cycles required per object visited.

The way that avoiding non essential conditionals has been addressed is to drop the NodeMask and TraversalMode parameters that are found in the OpenSceneGraph's osg
Node and osg::NodeVisitor respectively.  When NodeMask functionality is required in a scene graph the task will fall to a MaskGroup that will have a local mask and undertake the conditional during traversals that is required, so that only scene graphs that require a mask will pay the penalty for it.  Decision of what type of traversal to undertake is also moved to the Visitor subclass rather than the Visitor base class.  Finally the NodePath that is automatically accumulated by the OpenSceneGraph's NodeVisitor is also dispensed with, if Visitor implementation require this functionality then they are left to implement it.

To test the effectiveness of these design differences a test program, vsgroups (found in vsgFrameworks project) was used. The results of these seemingly small design changes over the OpenSceneGraph have a dramatic improvement in performance.

* quad tree construction and destruction times are 3 x faster in VSG vs OSG.
* quad tree traversal is 6 to 10 x faster in VSG vs OSG (depends upon Node and Visitor type.)

The reason for this dramatic improvement is due to:

* significant reduction in page faults due to memory footprint reduction
* reduction in conditionals, reducing number of instructions per object visited and improving CPU ability to prefetch and speculative execute
* improvement in number of instructions per cycle that the CPU can sustain

These are benefits even before we compared Vulkan vs OpenGL improvements, it's still too early in the projects life to be able to compare on realistic scenes, as things progress we'll provide more results.  We can be confident that the improvements in efficiency of the scene graph traversals combined with the efficiency of Vulkan will substantially improve the ability to have large and complex worlds, and reduce the power over-head required to achieve a specific level of visual quality.


## Productivity

The tools and middle-ware you choose for your projects are key determinants of the amount of work needed to achieve required functionality and to maintain and enhance that functionality through the software's life. The approach that we take within the VulkanSceneGraph project not only determines how productive it's own development is, it will have a great influence on how productive users of it will be. The following are principles that we are adopting to help us all achieve better productivity.

### General project development principles that aid Productivity:

* Use Best Practices that have been established in the wider industry:
    * [FOSS Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md) are used as a guide of how to organize and maintain the project
    * [CppCoreGuidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) are used as a guide to design and implementation
* Use C++17 to benefit from the improvements in C++ that result in cleaner and more maintainable code
    * Use features and idioms that make sense for a scene graph
    * Don't use features because they are new, trendy, or the "standard" way of doing things - they must make the code cleaner and more maintainable without hindering performance (i.e don't use std::shared_ptr<> as it has huge performance penalty.)
    * Don't make assumptions, implement and benchmark.
* Use CMake for cross-platform as it's effective and the de-facto standard build tool used in real-time graphics industry, when implementing scripts make them work in standard ways that developers will be familiar with
* Minimize dependencies for the VukanSceneGraph library to ensure it's quick and easy to build:
    * C++17
    * Vulkan
    * CMake
    * Native Windowing
* Use github as it's widely used, and structure the repository in ways that are familiar to others and work well with standard tools so no need to learn new tools & workflows
* Create documentation to help others use the VSG, and contribute to the VSG development
* Build a community to enable developers to help each other and coordinate testing, debugging and feature development, and to share best practice
* Work towards building a cohesive collection of companion libraries and high level frameworks on-top of the VSG to enable developers to use high level, domain specific frameworks and tools


### Scene Graph related principles that aid Productivity:

* ***"minimal and complete"*** : VulkanSceneGraph library to be focused on creating and traversing a scene graph and basic rendering in a viewer.
    * Additional libraries and frameworks to provide domain specific types of functionality, such as 3rd party data loaders to be provided by a family of supplementary libraries.
    * Developers to just pick the libraries they need from the family and only inherit the 3rd party dependencies they need for the project they have.
* Meaningful encapsulation of Vulkan
    * Vulkan is fast and flexible, but it is also long winded to implement basic functionality - it can take 1500 lines of code just to get a depth sorted quads on screen!
    * Simply wrapping Vulkan within C++ classes doesn't address what developers actually need
    * Developers need higher level functionality to manage their scenes and computational workflow
    * Need to encapsulate Vulkan in a way that reduces the work required to set up and use Vulkan
    * Encapsulation of Vulkan to fit coherently in with how it's used in a scene graph
    * Scene graph design and implementation must also fit with how Vulkan works - a symbiotic relationship
* ***""If it ain't broke, don't fix it."*** : A number of approaches used in OpenSceneGraph project remain relevant to the new scene graph:
    * Visitor pattern variation that couples type safe operation and scene graph traversals
    * Multi-pass, multi-stage rendering
    * Viewer, View and Camera and Windowing relationships
    * Intrusive reference counting
* Good baseline *Performance* opens the door to better *Productivity* : The lower overheads achieved with the new scene graph and Vulkan reduces the need for complex application level optimizations - to hit frame rate or latency targets less work needs to be done to workaround CPU bottlenecks previously associated with scene graph traversal and GL dispatch.

### Software quality principles that aid Productivity:

* Short cuts in code quality are *"Fools Gold"* when it comes to Productivity
* Take pride in clarity and efficiency of code
* Take time to understand and appreciate the code and techniques used by others - such as CppCoreGuidelines
* You want your work to be a *Success*, so write it like it'll *Live Forever* (and be looked at forever:-)
* If a problem exists, be honest about it, solve it right way and learn from the error and solution found
* *"Prepare to throw one away, you will anyhow"* (source : Mythical Man Month): don't get too wrapped up solving all the worlds problems in one place, solve the problems you have at hand and test the results, and if in the future it's found to be insufficient then refactor/rewrite it with everything you've learnt and now need to achieve
* Make use of static and dynamic analysis tools straightforward and a regular practice for developers and users


