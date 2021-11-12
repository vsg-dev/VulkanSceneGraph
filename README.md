![VulkanSceneGraph](https://raw.githubusercontent.com/vsg-dev/VulkanSceneGraph/master/docs/images/VSGlogo.png)

VulkanSceneGraph (VSG), is a modern, cross platform, high performance scene graph library built upon [Vulkan](https://www.khronos.org/vulkan/) graphics/compute API. The software is written in [C++17](https://en.wikipedia.org/wiki/C%2B%2B17), and follows the [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and [FOSS Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md).  The source code is published under the [MIT License](LICENSE.md).

The project aims to bring the performance of Vulkan to the wider developer community by providing a modern, high quality software library that is easy to use and focused on making the development of high performance graphics and compute applications a productive and fun experience.  Sharing the same lead author as the OpenSceneGraph, all the lessons about software quality, performance and the needs of application developers are applied to VulkanSceneGraph to provide a distillation of what a next gen scene graph needs to be.

This repository contains basic documentation, C++ headers and source and CMake build scripts to build the prototype libvsg library.  Additional support libraries and examples are provided in separate repositories, links to these are provided below.  The software currently builds under Linux, Windows, Android and macOS (using [MoltenVk](https://github.com/KhronosGroup/MoltenVK)).

## Features
The VulkanSceneGraph project is comprised of the main VulkanSceneGraph library (provided by this repo) and a collection of optional libraries, each in their own dedicated repositories hosted alongside each other on [https://github.com/vsg-dev](https://github.com/vsg-dev), that provide additional features and example programs and templates for your own VulkanSceneGraph projects.

### Features provided by the core VulkanSceneGraph library are:

* Robust, thread safe memory management with high performance smart pointers that are smaller and faster than std equivalents.
* GLSL style maths class - no need for 3rd party libs like GLM.
* Coherent Object model with easy to use and extend serialization, including native binary and ascii file support for all scene graph objects.
* C++ classes that encapsulate Vulkan Graphics and Compute C API in robust and convenient form, with robust resource management, including serialization support. Complexities and verbose setup usually associated with Vulkan are all dealt with for you so you can concentrate on your compute and graphics tasks.
* Vulkan RTX extensions for ray tracing and mesh shading.
* Class design focused on performance of scene graph operations by minimizing CPU bottlenecks: optimizing data density, layout, cache coherency and minimizing branching leading to better utilization of modern CPU and memory architectures. Traversals through to IO operations can be up to 10 times faster than the OpenSceneGraph.
* Optimized scene graph performance has been essential for making the most of the performance that Vulkan itself provides over OpenGL/DirectX, benchmarks on large databases show 3 to 20 X performance improvements over OpenSceneGraph/OpenGL.
* Multi-threading support at the viewer level, file loading and database paging.
* Flexible Viewer architecture built around Vulkan command recording and queue submission.
* Native windowing and event support under Windows, Linux, Android and macOS.
* Support for double matrices in Camera and Transform class providing support for large database coordinates system such as whole earth/GIS rendering whilst minimizing precision issues.
* Modern CMake build system that provides config installation alongside binaries making it easier to find and use all the appropriate build options for using the VulkanSceneGraph in your own projects.
* Minimal and complete approach to design - the whole VulkanSceneGraph interface and implementation, providing all the above functionality, takes 48 thousand lines of code, compared to over 58 thousand for GLM headers, or vulkan.hpp (C++ wrapper for Vulkan) at over 94 thousand lines of code.  The VulkanSceneGraph replaces both and provides much more functionality besides.

### Features provided by companion projects:
* [vsgXchange](https://github.com/vsg-dev/vsgXchange) reading and writing of 3rd party image and 3d models and HTTP support.
* [vsgGIS](https://github.com/vsg-dev/vsgGIS) integration with GDAL to adding support for Geospatial imagery/DEMs and coordinate transforms
* [vsgImGui](https://github.com/vsg-dev/vsgImGui) ImGui integration enabling UI in graphics window.
* [vsgQt](https://github.com/vsg-dev/vsgQt) - Qt integration with VulkanSceneGraph.
* [vsgUnity](https://github.com/vsg-dev/vsgUnity) plugin for Unity that provides export to native VulkanSceneGraph binary/ascii format.
* [vsgExamples](https://github.com/vsg-dev/vsgExamples) tests & examples.
* [MyFirstVsgApplication](https://github.com/vsg-dev/MyFirstVsgApplication) simple standalone VSG application that can be used as a template for your own applications.

## Useful links in codebase
* Detailed build and install [instructions](INSTALL.md)
* Headers - the public interface : [include/vsg/](include/vsg)
* Source - the implementation : [src/vsg/](src/vsg)
* Software development [Road Map](ROADMAP.md)
* Design : [Principles and Philosophy](docs/Design/DesignPrinciplesAndPhilosophy.md),  [High Level Decisions](docs/Design/HighLevelDesignDecisions.md)
* Community resources :  [Code of Conduct](docs/CODE_OF_CONDUCT.md), [Contributing guide](docs/CONTRIBUTING.md)
* Exploration Phase Materials (*completed*): [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md), [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md) and [Exploration Phase Report](docs/ExplorationPhase/VulkanSceneGraphExplorationPhaseReport.md)
* Prototype Phase Materials (*completed*): [Workplan](docs/PrototypePhase/Workplan.md) and [Prototype Phase Report](docs/PrototypePhase/PrototypePhaseReport.md)


## Public discussion list/forum
The VulkanSceneGraph Discussion Group [vsg-users](https://groups.google.com/forum/#!forum/vsg-users) is the place for project news, discussions of latest developments and any questions you have on how to use the software in your applications. The discussion group can be read by anyone, to post to the group you'll need register.

## Professional Support
Project lead, [Robert Osfield](mailto:robert.osfield@gmal.com), provides OpenSceneGraph and VulkanSceneGraph related services, the associated income supports open source development work and public support on both projects.  Services provided:

* Open source development - funding of development of new features specific to your project needs.
* Closed source bespoke development - development of proprietary libraries through to directly working on your applications.
* Confidential support via dedicated email discussion lists/IM/video conferencing for your team.
* Consulting.
* Training.

---

## Quick Guide to building the VSG

### Prerequisites:
* C++17 compliant compiler i.e.. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.7 or later.

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against older versions.  If you find success with older versions let us know and we can update the version info.

Download VulkanSDK from [LunarG](https://vulkan.lunarg.com/sdk/home), unpack into local directory and set VUKAN_SDK environment variable to the include/lib directory within it. For Linux it would typically be along the lines of:

    export VULKAN_SDK_VERSION=1.2.162.1
    export VULKAN_SDK=${PWD}/VulkanSDK/${VULKAN_SDK_VERSION}/x86_64

    mkdir VulkanSDK
    wget https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.gz -O VulkanSDK/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.gz
    tar zxf VulkanSDK/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.gz -C VulkanSDK/

Once you've downloaded and unpacked the VulkanSDK you'll want to put VULKAN_SDK into your user environment variable setup so that CMake's find_package(Vulkan) can find the VulkanSDK's location.

### Command line build instructions:

To build and install the static libvsg library (.a/.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    cmake .
    make -j 8
    sudo make install

Full details on how to build of the VSG (Unix/Windows/Android/macOS) can be found in the [INSTALL.md](INSTALL.md) file.
