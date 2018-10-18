VulkanSceneGraphPrototype (VSG) is a prototype for a modern scene graph library built upon Vulkan graphics/compute API.  The software is written in C++17, and follows the [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).  The source code is published under the [MIT License](LICENSE.md).

## Contents
* [Roadmap](#roadmap)
* [Prerequisites](#prerequisites)
* [Building the VSG](#building-the-vsg)
* [Using the VSG within your own projects](#using-the-vsg-within-your-own-projects)
* [Examples of VSG in testing/in use/how to work with VSG](#Examples-of-VSG-in-testing/in-use/how-to-work with-vsg)

## Roadmap

### 1. Exploration Phase, June-September 2018 (completed)
**Goal : Establsih which technologies and board techniques to use**

Learn and experiment with Vulkan, modern C++, and possible 3rd party dependencies.
Experimenting with different approaches to object/scene graph design and implementation

Resources associated with ExplorationPhase work:

* [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md)
* [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md)

### 2. Prototype Phase, October-December 2018 (present work)
**Goal : Rapid prototyping of main classes, library and test applications to establish how the scene graph API will broadly look and work.**

Develop as a throw away prototype, but keep in mind that not not of functionality is being prototyped, but scoping out the work practices that will work best once work on the final software starts.

### 3. Core Development Work, January-Summer 2019
**Goal: Implement the final class interfaces and implemetation**

Using the protyping work as a guide implement the final scene graph library with the aim of creatig a solid interface and implementation.

### 4. Release Work,  Fall 2019 onwards 
**Goal: Test scene graph library against real-world applications and shake down the API and implementation for it's first stable release.**

## Prerequisites
* C++17 complient compiler i.e. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.5 or later.
* [GLFW](https://www.glfw.org)  3.3 or later.  The plan is to implement native Windows so this dependency will
 later be removed.

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against oldeer versions.  If you find success with older versions let us know and we can related the version info.

## Building the VSG

Command line instructions for default building of static library (.a/.lib) in source:

    git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake .
    make -j 8
    make install

Command line instructions for building shared library (.so/.lib + .dll) in out of source:

    git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git
    mkdir vsg-shared-build
    cd vsg-shared-build
    cmake ../VulkanSceneGraphPrototype -DBUILD_SHARED_LIBS=ON
    make -j 8
    make install

## Using the VSG within your own projects

The project is currently a prototype that is undergoing continuous development so it isn't recommend to use as base for long term software development. At this point it's available for developers who want to test the bleeding edge and provide feedback on it's fitness for purpose.

To assist with setting up software to work with the VSG we provided [FindVSG.cmake](https://github.com/robertosfield/VulkanSceneGraphPrototype/blob/master/CMakeModules/FindVSG.cmake), [FindGLFW.cmake](https://github.com/robertosfield/VulkanSceneGraphPrototype/blob/master/CMakeModules/FindGLFW.cmake) and [FindVulkan.cmake](https://github.com/robertosfield/VulkanSceneGraphPrototype/blob/master/CMakeModules/FindVulkan.cmake) within the [VullkanSceneGraphPrototype/CMakeModules](https://github.com/robertosfield/VulkanSceneGraphPrototype/tree/master/CMakeModules) directory.  Feel free to copy these into your own project.

Within your CMake CMakeLists.txt script to find the VSG related dependencies you'll need to add:

   find_package(VSG)
   find_package(GLFW)
   find_package(Vulkan)

To select C++17 compilation you'll need:

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

To link your lib/application to required dependnecies you'll need:

    target_link_libraries(mytargetname VSG::VSG GLFW::GLFW Vulkan::Vulkan)

This will tell CMAke to set up all the appropriate include paths, libs and any definitions (such as the VSG_SHARED_LIBRARY #define that is required under Windows with shared library builds to select the correct declspec().)

## Examples of VSG in testing/in use/how to work with VSG

It's still very early days for the project so we don't have many projects that link to the VSG to reference, for our own testing purposes we have two project which may serve as an assistance to how to compile against the VSG and how to use parts of it's API.  These projects are:
* [osg2vsg](https://github.com/robertosfield/osg2vsg)
* [vsgFramework](https://github.com/robertosfield/vsgFramework)

Three examples within the vsgFramework project that may be of paricular interest are ports of Vulkan tutorials to the VSG api.  In each case the VSG version requires less than 1/5th the amount of code to achieve the same functionality.
* [Vulkan Tutorial](https://vulkan-tutorial.com/)  ported as [vsgFramework/applications/vsgdraw](https://github.com/robertosfield/vsgFramework/blob/master/applications/vsgdraw/vsgdraw.cpp)
* Version of vsgdraw using vkPushConstants [vsgFramework.applications/vsgpushconstants](https://github.com/robertosfield/vsgFramework/blob/master/applications/vsgpushconstants/vsgpushconstants.cpp)
* [vulkan_minimal_compute](https://github.com/Erkaman/vulkan_minimal_compute) tutorial ported to VSG [vsgFramework/applications/vsgcompute](https://github.com/robertosfield/vsgFramework/blob/master/applications/vsgcompute/vsgcompute.cpp)


