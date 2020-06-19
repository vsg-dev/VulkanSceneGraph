# VSG Exploration Phase Report
This document discusses the work carried out during the Exploration Phase and conclusions and results from this work.  Links to 3rd party resources used are included inline with this document, click on highlighted keywords to follow links.

## Original Plan for topics to be tackled during exploration phase:
1. Gain familiarity and experiment with Vulkan API and associated tools such as glslang, SPIR-V Tools etc.
2. Develop an intuitive, flexible and high performing conceptual and class mapping for Vulkan functionality into the Application and Scene Graph domains.
3. Experiment with different approaches to core scene graph design and implementation to find out which approaches deliver the best balance between performance, flexibility and scalability, whilst providing a clean and intuitive design and implementation.
4. Explore the scope of functionality to be placed in the core VulkanSceneGraph library/project and functionality that should be provided by additional/3rd party libraries or frameworks.
5. Gain familiarity and experiment with C++11, C++14 and C++17 features and make choices which provide the best balance between a clean scene graph design, implementation and compiler compatibility.
6. Test build tool options CMake and xmake to inform decision which tools to use.


The document will discuss the work and findings from each of these areas, numbering of sections follows the above guide from the original plan. A final section in this document will provide:


7. [Exploration Conclusions](#7-exploration-phase-conclusions)
8. [High Level Design Decisions](../Design/HighLevelDesignDecisions.md)

## VulkanPlayground repository and successors:
To provide a base for experimental work during the exploration a project was created on github : [VulkanPlayground](
https://github.com/vsg-dev/VulkanPlayground) which has been kept a private repository.

The VulkanPlayground is meant as a throwaway prototyping repository rather than an alpha version of the final scene graph project. The project contains:

* prototype vsg library that contains:
    * core : base classes and templates, such as Object, Array, ref_ptr
    * nodes : scene graph classes  Node, Group, StateGroup, LOD classes
    * maths: vec2, vec3, vec4 and mat4 template classes
    * vk : Vulkan wrapper classes
   * viewer :  Viewer, Window and GraphicsStage classes
* prototype osg2vsg utility library for loading images using the OSG and converting to vsg/Vulkan objects
* Test-bed applications that experiment with various aspects of the vsg prototype

Now that the Exploration Phase is completed the work on VulkanPlayground has fed into the Prototype Phase, with the repository being broken up into three component repositories that are publicly available and public under the MIT License:

* [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph) the core scene graph
* [osg2vsg](https://github.com/vsg-dev/osg2vsg) helper library to read/writes images using the OSG
* [vsgFramework](https://github.com/vsg-dev/vsgFramework) experiment with building applications and libraries


---

## 1. Gain familiarity and experiment with Vulkan API and tools
The process of familiarization with Vulkan and associated tools was progressed by working through the VulkanTutorial and vulkan_minimal_compute tutorials, remapping the functionality that these tutorials provided into reusable C++ class wrappers for Vulkan functionality.  The Vulkan Programming Guide book and Khronos online reference guides for Vulkan were used to fill out knowledge of the API and how it functioned.


GLFW was used for creating Vulkan windows and glslang used to convert GLSL Vulkan compatible shaders into SPIRV .spv shaders usable by Vulkan.


The work on following the VulkanTutorial can be found in the vsgdraw example, and vulkan_minimal_compute can be found in the vsgcompute test-bed applications. Both of these applications provide similar functionality as the original examples, but do so by using the prototype vsg classes. The benefit from the vsg classes is that the vsgdraw.cpp and vsgcompute.cpp are less than 1/5th the size of the original tutorials that they are mapped from. The vsg versions are also more linear in their layout and should be easier to follow than the originals that they recreate.


The vsgdraw test-bed using some basic graph functionality it is very simplistic compared to what the final scene graph will provide - there is no culling, a graph is used just to hang state and command functionality in the form of a command graph that is traversed to dispatch to Vulkan.


The vsgcompute test-bed uses the vsg Vulkan wrappers directly as immediate mode, setting up and dispatching data and commands to Vulkan directly.

---

## 2. Develop class mapping for Vulkan functionality
Two of the three months of the Exploration Phase have been dedicated to learning and experimenting with Vulkan. Vulkan requires a great deal of setup to do basic things so progress in this area was been slow. The current Vulkan encapsulation that can be found in VulkanPlayground/include/vsg/vk and src/vsg/vk are functional and usable as is, but should be considered a first pass implementation.


The current encapsulation of Vulkan has followed the principle of one C++ class to each key Vulkan object type, so VkPipeline is wrapped up in vsg::Pipeline class found in include/vsg/vk/Pipeline etc.


Initial work has been done on exposing the Vulkan functionality within the scene graph and viewers. This work is ongoing and will not be resolvable within the original three month Exploration Phase. Areas in Vulkan left to be resolved are how multi-threading and multi-device support will be handled, and by what means the Vulkan objects will be connected to the scene, command graphs and viewers.

---

## 3. Exploration of core scene graph design and implementation
The core functionality such as memory management, type safe object operations, extensible object properties, maths functionality and memory footprint were fleshed out in a series of classes within the prototype vsg library and the application test-beds. The general approach has been to create core classes that are smaller in memory footprint, more flexible and coherent than their counterparts within the OpenSceneGraph.

### 3.1 Efficient memory management
The smaller memory footprint is a key part of addressing the memory bandwidth that is the main bottleneck for scene graph traversals.  Several approaches have been tested to address memory footprint and cache coherency:


* Move properties that aren’t used on all Objects out into an optional Ancillary class
* Provided option for using fixed sized arrays to improve cache coherency


The osggroups test-bed application provides a comparison of different approaches within the VSG as well as comparing to OSG equivalents testing creation, destruction and traversal of quad tree test scene graphs. This test-bed illustrates how time to create, delete and traverse are all related to the memory footprint. Findings are:


* Fixed sized vsg::QuadGroup requires 56 bytes vs 264 for osg::Group with 4 children
* vsg::QuadGroup, time to create is 1/5th of osg::Group with 4 children
* Traversals can be up to 10 times faster than OpenSceneGraph


Smart pointer usage also has a profound effect on memory footprint and hence performance. The osgpointer test-bed compares use a intrusive reference counting (vsg::ref_ptr) vs using C++11’s std::shared_ptr.  On 64 bit Linux systems vsg::ref_ptr<> is 8 bytes in size vs std::shared_ptr<> that is 16 bytes.  The internal nodes of a scene graph use smart pointers to hold references to their children so also are impacted by the smart pointer size.  vsg::QuadGroup using ref_ptr<> is 56 bytes vs 104 bytes required for the equivalent fixed size Group using shared_ptr<>. This test illustrates how std::shared_ptr<> is a significant step back compared to the intrusive reference counting and should not be used in any performance sensitive areas of the VSG project.

### 3.2 Improving Flexibility and coherence
The OpenSceneGraph’s long life has meant that features have been added over time with multiple class hierarchies being used for different purposes. For instance the scene graph is distinct to the rendering back-end graphs, uniforms are different to arrays, traversal and type safe operations on object also have different mechanisms.


To address traversal and type safe operations in a more generic way the osg::NodeVisitor (and other equivalents within the OSG) are replaced by a single vsg::Visitor base class that all vsg::Objects can be interfaced with.


Uniforms and vertex arrays are also supported using the same vsg::Data base class with vsg::Value and vsg::Array template classes to provide wrapping of single value or arrays of values respectively.


The lightweight nodes within the scene graph also mean that the cost of creating companion graphs is lower so there is no need for specialized graphs as is done with the OSG. The same node classes can now be used for both the main scene graph and the rendering back-end which is done with a command graph - which is essentially a scene graph used to hold data and commands that will be dispatched into the Vulkan Command Buffers.

---

## 4. Scope of core VulkanSceneGraph vs 3rd party libraries
There are a range of 3rd party libraries that could be useful and a range of ways that they might be integrated:

* used as external dependencies
* included within VSG source code as git submodules
* cherry-picked source code or
* used as inspiration for design and implementations written as original work within the VSG project


Possible areas where 3rd party libraries could be utilized include:

1. Maths
2. Windowing
3. Vulkan wrappers


Each of these areas we review the 3rd party libraries to look at their relevance and usefulness, and where useful how best to use them.  As a general principle external dependencies can reduce the amount of work required in the core VSG project, but increase the work required to assemble the required dependencies and has the potential for creating an incoherent user experience with different dependencies using their own design style and tools.


Creating this same functionality directly ourselves offers the opportunity of creating a coherent design for all features and minimizing the work required for assembling dependencies, with the downside that all locally implemented features must be designed, implemented, tested, debugged and maintained ourselves.

### 4.1 Maths
To improve the coherence between the VSG and Vulkan’s use of GLSL the plan is to use the same naming and conventions as GLSL. The GLM library fulfils this goal so has been reviewed with consideration of using it as 3rd party dependencies.


GLM is well established, this is both a positive and a negative. It is likely to be well tested across platforms and should be reliable, it’s design and implementation follow GLSL equivalents very closely to a high level providing a coherent experience between the C++ application domain and the shaders passed to the graphics hardware.


The disadvantage of GLM is that is very large, 46913 lines of code in headers, and the majority of it’s functionality will be rarely used by a scene graph user. GLM is also written for OpenGL, while GLSL is usable with Vulkan, the application level elements and conventions are not all compatible. The depth range and vertical orientation of clip space are different between OpenGL and Vulkan so require GLM results to be adapted so they can be used with Vulkan - the projection matrix setup is an example of this.


GLM is used by a number of Vulkan based projects, for instance NVidia’s VkHLF, vulkan-cpp-library pumex, the VulkanTutorial all use GLM.


When considering whether to create local classes vs using 3rd party dependencies a key aspect is just how much work would be required to create the subset of functionality that the VSG requires. The key elements for the VSG are vec2, vec3, vec4 and mat4 classes, so as an experiment VSG template classes for each of these were implemented, enabling the standard float variations as well as double and integer versions are very local cost. GLM provide a few design/implementation pointers that helped in this work. The total code base for this functionality is presently just 429 lines of code (found in VulkanPlayground/include/vsg/maths). This is 1/100th of the code base of GLM.  These locally created classes were also written to be directly compatible with Vulkan’s clip space conventions so no application level adaptation is required.


The prototype maths classes provided in vsg/maths are still very basic, it’s likely that the total code base dedicated to this will need to more than double in size. It will however remain well below the footprint of GLM. Experience with maths classes in the OSG suggests that once written they tend to be very easy to maintain so handling this functionality within the VSG project will not be burdensome.


For the VSG project is looks best to provide our own maths classes, it gives us the ability to be fully coherent with how Vulkan works and with the conventions that will be used in the rest of the VSG, and provides a small code footprint for users to navigate and learn, and avoids adding a large 3rd party dependency.


### 4.2 Windowing
The work carried out in replicating the VulkanTutorial used the same GLFW library the the VulkanTutorial uses to create a Window and associated Vulkan surface. GLFW is a C library and requires initialization and clean up in a particular order controlled at the application level.


To make the window creation and clean up easier GLFW_Window and GLFW_Instance classes were written to provide a C++ interface and an automatic means of clean up, decoupling the test-bed applications from having to handle this task. This functionality was eventually wrapped up inside the prototype vsg library completely so the public vsg interface is now entirely agnostic of windowing library used to create the windows. The total GLFW codebase is presently 37,246 lines of code. GLFW is licensed under zlib License.


Another Windowing library that provides Vulkan support is WSI-Window. This is written specifically for Vulkan in C++ and has Windows, Linux and Android support. Feature wise WSI-Window is a possibility, however, the style of WSI-Window interface is not coherent with Vulkan, or VSG work so far, and some elements of the implementation are somewhat odd. WSI-Window codebase is currently 3,679 lines of code. WSI-Windows is licensed under Apache License.


Another reference for Windowing is pumex (a C++ rendering based framework based on Vulkan). It provides it’s own local Windows and Unix windowing implementations which are very small - just 423 lines of code for Win32, and 403 for Xcb (X11/Unix). Pumex is licensed under the MIT License.


Paweł Księżopolski, the author of Pumex, is a previous contributor to the OpenSceneGraph project and I believe remains an OpenSceneGraph user in his professional career. Pumex is probably the closest any 3rd party project has come to delivering what the VSG aims to provide, so is technically a competitor, but I am optimistic that Pawel will view our work on VSG favourably and may wish to collaborate and share work.


The small size of WSI-Window and in particular the tiny size of Windowing support in pumex provides encouragement that implementation native Windowing within the VSG will not be a large task. We can either learn form or possibly even share code directly for the implementation side.


To provide the most coherent user experience the approach for the VSG will be:

* Provide a platform agnostic public interface to creating/destroying Windows and handling events
* Provide native windowing implementations for all the major platforms - the implementations would be internal to the VSG library, either directly with source code or by linking to 3rd party libraries. First pass would be linking to 3rd party library, then moving source code internally to keep dependencies simple.
* Public interface to Windowing and Events need to be adaptable to 3rd party windowing libraries such as Qt etc.

### 4.3 Vulkan wrappers
The main Vulkan headers are all C headers that contain functions to create and destroy objects and functions dispatch commands, as well as structs used to pack properties used to setup the Vulkan objects and control the commands, queues etc. Using Vulkan C headers directly can result in large amount of setup code and careful management of the lifetime of resources.


There are a series of C++ headers/libraries that encapsulate the Vulkan C objects and functions and provide additional type safety or features. Each of these C++ wrappers have their own advantages and disadvantages.


The vulkan.hpp header is an auto-generated C++11 compatible wrapper for vulkan.h. To quote directly the description of vulkan:


“The goal of the Vulkan-Hpp is to provide header only C++ bindings for the Vulkan C API to improve the developers Vulkan experience without introducing CPU runtime cost. It adds features like type safety for enums and bitfields, STL container support, exceptions and simple enumerations.”


The vulkan.hpp in the 1.1.82.0 release of the VulkanSDK is 45177 lines of code.  This single header is so large that github reports “(Sorry about that, but we can’t show files that are this big right now.)”. All the classes that this header provide are in this single header, this in exact opposition to widely adopted best practice for C++ of having a single class per header.


For the huge size of vulkan.hpp there is few really compelling features added over the C API. There is some primitive memory management support but no where near sufficient for the purpose of serious application or scene graph development. To use Vulkan within the scene graph we still need to add this coherent memory/resource management - we still need to wrap the Vulkan objects, so if one uses vulkan.hpp you have two extra levels of wrapper and indirection for the underlying Vulkan objects and functions that are doing the work.


Managing complexity of design and implementation is of key importance for all software projects, adding complexity should only ever be done when it adds value that justifies it. Vulkan.hpp performs poorly by this metric and does not justify itself for use in the VSG project.


The vulkan-cpp-library was also considered. This is C++11 library that uses the Apache License and authored by an Google employee as their own project. The project has laid dormant for 2 years. The class naming and coding style takes notes far more from the C++ standard library than Vulkan that it wraps. This approach means that resulting code breaks with the style of all Vulkan headers and documentation, this incoherence is really jarring. This project is clearly an experiment that was dropped by the author before it was complete and no one else has come along to pick it up to finish it or maintain it.


The VkHLF (Vulkan High Level Framework) is a C++11 wrapper for Vulkan that builds upon vulkan.hpp adding better memory management and other facilities. VkHLF is developed by NVidia is a up to date and looks to be actively maintained. The class naming and style is also coherent with Vulkan so it’s relatively easy to relate VkHLF code to underlying Vulkan C API and Vulkan documentation that is predominately relates to the Vulkan C API.  VkHLF uses a NVidia drafted LICENSE that looks similar in principle to the MIT LICENSE.


The VkHLF is a serious body of work but still quite modest in size - 3,633 lines of code in the headers and another 5,142 lines of code in implementation. However, it depends upon the vulkan.hpp C++ bindings, so we have vkhlf::Instance (from vkhlf/Instance.h) wrapping a vk::Instance (from vulkan/vulkan.hpp) wrapping VkInstance from vulkan_core.h. This tells us vulkan.hpp is flawed - it simply doesn’t provide enough useful functionality to be useful on it’s own, so VkHLF adds some of those missing features.


However, design and implementation wise it’s simply not a good practice - working around flaws in a 3rd party body work functionality by building upon that flawed body of work. It may resolve some of the flaws but it’s still built upon a flawed foundation. You don’t build upon a sandy beach and expect your your building to remain robust long term.


The existence of VkHLF shout out that what if Khronos want to provide a C++ wrapper to Vulkan then it should be in the form of VkHLF without any extra levels of auto-generated headers in between. Perhaps in the future Khronos will do just this, but at this point in time VkHLF is a step in the wrong direction, it’s building upon sand (vulkan.hpp) not rock (vulkan.h).


For a scene graph the Vulkan objects and functions need to be created and invoked in specific ways that make sense for the scene graph and the applications that build upon it. For a scene graph wrapping Vulkan in a C++ API is not it’s primary purpose, the primary purpose is efficiently passing data to graphics hardware to be processed by the GPU. Extra facilities that make usage in the context of a scene graph easier don’t exist in a general purpose C++ wrapper for Vulkan, so you’d need to add them, and when you do you add an extra layer of classes and objects. One has to be careful how you wrap Vulkan, if done well it works efficiently and adds clarity of how the functionality relates to the underlying API, if done badly it adds memory or computation overhead and obfuscated what the software is doing.


The pumex project has also tackled this same issue - how to wrap up Vulkan functionality in the context of a scene graph. The approach that Pawel has taken is to use the vulkan.h C API wrapping selected features with pumex classes named in a coherent way to the underlying Vulkan features, so VkDevice maps to pumex::Device. The Vulkan C API is a well designed and easy to follow API - it’s very verbose, but it’s coherent, the layers you need on-top to make it useful to a C++11 scene graph are actually quite lightweight.


Pumex is a rendering library in it’s own right, it’s not a Vulkan wrapper, it has basic scene graph functionality already provided - elements of which are reminiscent of the OpenSceneGraph that reveal it’s author's long exposure to the OSG. Pumex can be thought of as a prototype for the VSG project rather than a 3rd party library that the VSG library would build upon. It illustrates nicely that wrapping Vulkan ourselves need not be an significant task, and offers opportunities to build a coherent bridge between the C++11 application domain and the lower level C domain that Vulkan works within.


The VulkanPlayground work experimenting with wrapping Vulkan is not based on pumex, rather it’s a based of incrementally recreating the VulkanTutorial functionality in a series of C++ wrappers for Vulkan objects. The wrappers are all located in VulkanPlayground/include/vsg/vk. The vsg namespace is used so VkDevice maps to vsg::Device.  The granularity of the approach is similar to what pumex uses but completely independently derived, with interface and implementation which are far more minimal in the vsg equivalents.  The vsg wrappers focus on creation, automatic resource clean-up and memory management. This is only prototype work so focus on key functionality rather than completeness of API and implementation.


The vsg/vk headers now total 1,839 lines of code, while the vsg/vk implementations total 2,337 lines of code for a total of 4167 lines of code. This is slightly less than half the size of VkHLF headers and source, and less than 1/10th the size of vulkan.hpp C++11 headers.  Despite the vsg/vk wrappers for Vulkan being a fraction of the size of vulkan.hpp they are far more useful for the purpose of creating a scene graph. The naming conventions have been kept coherent with the underlying vulkan.h C API and were possible the C structs and enums can be used directly. The prototype work done in VulkanPlayground illustrate how providing our own Vulkan wrappers is the best way to provide lightweight, coherent and useful encapsulation of Vulkan.

---

## 5. Gain familiarity and experiment with C++11, C++14 and C++17
The VulkanPlayground has adopted C++11 from the start, both the application test-beds and the vsg prototype library have been used to trial various C++11 features. C++11 is huge step forward for C++ programmers, enabling code to be cleaner, more succinct and more robust.

Not all features of C++11 are useful for scene graphs - testing of std::shared_ptr<> found that it’s memory overhead compared to locally provided intrusive reference counting is prohibitive and precludes its use in the context of a scene graph where memory footprint and bandwidth are key bottlenecks.

There has not been time during the Exploration Phase to experiment with C++14 and C++17 - work on learning and experimenting with Vulkan has taken precedence. At this point in time it’s clear that C++11 is very useful and sufficient for a major step forward in scene graph development. Whether C++14 and C++17 will probably crucial features is not something that can be established without spending time evaluating them.

Notes for September Extension of Exploration Phase: explored C++14 and C++17 and found that features in C++17 offer cleaner and more compact code that is easier to read and maintain. The memory allocator and filesystem features of C++17 are useful additions but at this point in time clang and gcc compiler support is experimental, only VisualStudio has full support. The improvements in code clarity alone justify adoption of C++17 going forward.

---

## 6. Test build tool options CMake and xmake.
Familiarity with Cmake made it an easy choice for the first pass of work on VulkanPlayground. There hasn’t been sufficient time to look at xmake within the three month Exploration Phase so it hasn’t been possible to evaluate the pros and cons of CMake vs xmake for the final VSG project.


As a general comment, all of the 3rd party projects and all of the Khronos toolsets reviewed during this phase use CMake. All OpenSceneGraph users will also be familiar with CMake. Market penetration of CMake within the computer graphics developer community makes it an uncontroversial choice.

For xmake to be adopted it will need to offer benefits for VSG developers and users to justify introduction of an unfamiliar tool. One way to evaluate xmake would be to port the present VulkanPlayground project from CMake to xmake.  If required this can be done after the completion of the present Exploration Phase.

---

## 7 Exploration Phase Conclusions
The Exploration Phase has covered most of key areas of investigation outlined in the original plan for this phase. Vulkan while well designed is verbose and complex to work with so has taken the majority of the available time to explore, to an extent that there has been insufficient time to research use of C++14, 17 and xmake within the scope of the 8 weeks work available. The one month extension to the Exploration Phase focused on C++17 and confirmed as appropriate version for final VSG.


A range of 3rd party maths, windowing and vulkan wrappers were reviewed as means of learning what is possible and for consideration as a 3rd party dependency. In the area of Maths GLM is a possibility but it’s implementation is messy and sprawling and supports GL rather than Vulkan so isn’t a perfect fit. Implementing our own GLSL style, Vulkan centric maths classes is a straightforward task so the need for GLM to minimize our own work effort is not compelling enough to justify it as a 3rd party dependency.


VkHLF is the best of the C++ wrappers of Vulkan but builds upon the autogenerated vulkan.hpp wrapper of vulkan C API, that is so large that standard developer tools like github fail to handle it as normal C++ header. VkHLF also creates a double wrapping of Vulkan classes, something that is a crude means of compensating for the lack of useful functionality that the Vulkan C++ header provides. The final C++ class Vulkan wrappers that VkHLF provides, while higher level than the Vulkan C API, still falls short of what is required to make the Vulkan objects directly usable within a scene graph.

A key part of the work in this phase has been focused on learning Vulkan and to this end creation of C++ wrappers for key Vulkan objects directly using the Vulkan C API provided a way of testing Vulkan and how best to manage it in C++ and within a scene graph. The Vulkan C API is well designed and favours wrapping in C++ objects that add resource management. The general approach has been to take a Vulkan object like VkDevice and map to a vsg::Device class.

Recreating the the VulkanTutorial was done using these C++ wrappers and has enabled a reduction in code size from 1530 lines to 275 lines in the vsgdraw.cpp test-bed. A similar code size reduction was achieved with the porting of the vulkan_minimal_compute tutorial to use these vsg Vulkan wrappers - 805 lines down to 141 lines.


Work has begun on adapting the Vulkan wrappers to work with the needs of a general purpose scene graph.  his work has not been completed, there has simply been too much work required to tame Vulkan to complete this experimental work within the 3 month time frame. This means parts of the VSG design is still open ended.


Windowing has not been a major focus during this phase, GLFW has been used as it provides an easy means for creating a Vulkan capable Window and Surface on which vulkan can be rendered with. GLFW was used primarily because the VulkanTutorial and other tutorial code use it, rather than using this to evaluate it’s suitability for VSG to use as it’s main mains for creating windows. The pumex project has its own windowing support that while more limited than GLFW is small and entirely focused on Vulkan rather than a GL windowing library that has been adapted to support Vulkan as well. The small size of the code required to providing windowing and event handling in pumex shows that handling native windowing within the VSG project will not be a significant challenge. Providing native windowing support ourselves will provide a coherent public interface and avoid adding external dependencies. Pumex is an open source project under the MIT license so sharing code is also a possibility.


As a general finding, the 3rd party dependencies reviewed have all provided useful insight into how or not to implement various features, ultimately none are useful enough directly to justify using as a direct 3rd party dependency, in the areas of maths, vulkan integration and windowing we can provide our own classes that are coherent with each other and tuned to the requirements of use with a scene graph and graphics applications that build upon them.

