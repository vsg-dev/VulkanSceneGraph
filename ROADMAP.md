## Roadmap

### 1. Exploration Phase, June-September 2018 (completed)
**Goal : Establish which technologies and broad techniques to use**

Learn and experiment with Vulkan, modern C++, and possible 3rd party dependencies.
Experimenting with different approaches to object/scene graph design and implementation. Exploration Phase Materials :

* [Principles and Philosophy](docs/Design/DesignPrinciplesAndPhilosophy.md)
* [High Level Design Decisions](docs/Design/HighLevelDesignDecisions.md)
* [Exploration Phase Report](docs/ExplorationPhase/VulkanSceneGraphExplorationPhaseReport.md)
* [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md)
* [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md)

### 2. Prototype Phase, October-December 2018 (completed)
**Goal : Rapid prototyping of main classes, library and test applications to establish how the scene graph API will broadly look and work.**

Prototype Phase Materials:

* [Prototype Phase Workplan](docs/PrototypePhase/Workplan.md)
* [Prototype Phase Report](docs/PrototypePhase/PrototypePhaseReport.md)

### 3. Core Development Phase, 2021-early 2022 (near completion)
**Goal: Create the final class interfaces and implementation**

Using the prototyping work as a guide implement the final scene graph library with the aim of creating a solid interface and implementation.

#### Completed tasks:
* Development of final VSG Library.
* Support for multi-threaded database paging.
* Support for multi-threaded viewer, cull and dispatch traversals.
* Support for multi-pass rendering.
* Support for large scale whole world databases, with double support for scene graph transforms.
* Development of add on libraries that provide:
    * Support for major image formats.
    * Support for major 3D model formats, including FBX, glTF.
    * Support for PBR shaders.
    * Support for Text rendering.
* Development of test suite of programs and data.
* Support for RTX Mesh shaders and ray tracing.
* Scene graph level multi-bin support with bin sorting.
* Support for Khronos ray tracing.
* Memory allocator with support with grouping associated types
* Positional state support to enable easier support of lighting, shadows, texture projection.
* Matrix decomposition
* Port to iOS
* Unified state composer, shader set and cache
* Update vsgXchange::Assimp to use the new state composer/shader set.
* Support for wide and standard strings in vsg::Path.
* Utilize vkEnumerateInstanceVersion
* Update vsgImGui to latest
* Improved support for dynamic scene graphs and dynamic views, dynamic descriptor pool reallocation

#### Current development tasks:

### 4. Release Phase,  Spring - Fall 2022
**Goal: Test scene graph library against real-world applications and shake down the API and implementation for it's first stable release.**

* Refinement of API and implementation
* Build relationships with application developers and involve them in testing
* Create tutorial and example programs to illustrate how to use VSG
* Test, debug, refine and release 1.0.0!

#### Future tasks relating to associated libraries
* Update vsgXchange::OSG to use the new state composer/shader set.
* Rewrite vsgQt handling of keyboard mapping
* Support for integration with OpenGL/OSG applications via [EXT\_external\_object](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_external_objects.txt) & [VK\_KHR\_external\_memory](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VK_KHR_external_memory.html#versions-1.1-promotions)
