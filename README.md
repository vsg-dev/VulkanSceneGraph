| [View as Website](https://vsg-dev.github.io/VulkanSceneGraph/) |  [View as GitHub repository](https://github.com/vsg-dev/VulkanSceneGraph) |


VulkanSceneGraph/VkSceneGraph (VSG), currently under development, is a modern, cross platform, high performance scene graph library built upon [Vulkan](https://www.khronos.org/vulkan/) graphics/compute API. The software is written in [C++17](https://en.wikipedia.org/wiki/C%2B%2B17), and follows the [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and [FOSS Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md).  The source code is published under the [MIT License](LICENSE.md).

The project aims to bring the performance of Vulkan to the wider developer community by providing a modern, high quality software library that is easy to use and focused on making the development of high performance graphics and compute applications a productive and fun experience. Our aim is reach a standard usable professional compute and graphics applications in the second half of 2019.

This repository contains basic documentation, C++ headers and source and CMake build scripts to build the prototype libvsg library.  Additional support libraries and examples are provided in separate repositories, links to these are provided below.  The software currently builds under Linux, Windows, Android and macOS (using [MoltenVk](https://github.com/KhronosGroup/MoltenVK)). Support for iOS will be added in 2019.

## Public discussion list/forum
The VulkanSceneGraph Discussion Group [vsg-users](https://groups.google.com/forum/#!forum/vsg-users) is the place for project news, discussions of latest developments and any questions you have on how to use the software in your applications. The discussion group can be read by anyone, to post to the group you'll need register.

## Useful links in codebase and to associated projects
* Detailed build and install [instructions](INSTALL.md)
* Headers - the public interface : [include/vsg/](include/vsg)
* Source - the implementation : [src/vsg/](src/vsg)
* Tests & Examples - companion repository : [https://github.com/vsg-dev/vsgExamples](https://github.com/vsg-dev/vsgExamples)
* Software development [Road Map](ROADMAP.md)
* Design : [Principles and Philosophy](docs/Design/DesignPrinciplesAndPhilosophy.md),  [High Level Decisions](docs/Design/HighLevelDesignDecisions.md)
* Community resources :  [Code of Conduct](docs/CODE_OF_CONDUCT.md), [Contributing guide](docs/CONTRIBUTING.md)
* Exploration Phase Materials (*completed*): [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md), [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md) and [Exploration Phase Report](docs/ExplorationPhase/VulkanSceneGraphExplorationPhaseReport.md)
* Prototype Phase Materials (*completed*): [Workplan](docs/PrototypePhase/Workplan.md) and [Prototype Phase Report](docs/PrototypePhase/PrototypePhaseReport.md)

---

## Quick Guide to building the VSG

### Prerequisites:
* C++17 compliant compiler i.e.. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.7 or later.

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against older versions.  If you find success with older versions let us know and we can update the version info.

### Command line build instructions:
To build and install the static libvsg library (.a/.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    cmake .
    make -j 8
    make install

Full details on how to build of the VSG (Unix/Windows/Android/macOS) can be found in the [INSTALL.md](INSTALL.md) file.

---

## Examples of the VSG in use

It's still very early days for the project so we don't have many projects that use to the VSG to reference, for our own testing purposes we have two project which may serve as an illustration of how to compile against the VSG and how to use parts of it's API.  These projects are:

* [vsgExamples](https://github.com/vsg-dev/vsgExamples) example programs that we are using to test out VSG functionality and illustrates usage.
* [osg2vsg](https://github.com/vsg-dev/osg2vsg) utility library that integrates OpenSceneGraph with the VSG to leverages 3d model and image loaders and converts them to VSG equivalents.  Once converted they can be viewed with the osg2vsg application, or loaded and rendered by [vsgviewer](https://github.com/vsg-dev/vsgExamples/tree/master/Desktop/vsgviewer) provided by vsgExamples.

Three examples within the vsgExamples project that may be of particular interest are ports of Vulkan tutorials to the VSG API.  In each case the VSG version requires less than 1/5th the amount of code to achieve the same functionality.

* [Vulkan Tutorial](https://vulkan-tutorial.com/) ported as [vsgExamples/Desktop/vsgdraw](https://github.com/vsg-dev/vsgExamples/blob/master/Desktop/vsgdraw/)
* [vulkan_minimal_compute](https://github.com/Erkaman/vulkan_minimal_compute) tutorial ported to VSG [vsgExamples/Desktop/vsgcompute](https://github.com/vsg-dev/vsgExamples/blob/master/Desktop/vsgcompute/)

