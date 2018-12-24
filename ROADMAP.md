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

### 3. Core Development Phase, January-Summer 2019
**Goal: Create the final class interfaces and implementation**

Using the prototyping work as a guide implement the final scene graph library with the aim of creating a solid interface and implementation.

* Development of final VSG Library
* Support for multi-threaded database paging
* Support for multi-threaded viewer, cull and dispatch traversals
* Support for multi-pass rendering
* Support for large scale whole world databases (double support for scene graph transforms)
* Development of add on libraries that provide:
    * Support for major image formats
    * Support for major 3D model formats, including FBX, glTF.
    * Support for PBR shaders
    * Support for Text rendering
* Development of test suite of programs and data
* Support for RTX Mesh shaders and ray tracing
* Port to iOS

### 4. Release Phase,  Fall 2019 onwards
**Goal: Test scene graph library against real-world applications and shake down the API and implementation for it's first stable release.**

* Refinement of API and implementation
* Build relationships with application developers and involve them in testing
* Create tutorial and example programs to illustrate how to use VSG
* Test, debug, refine and release 1.0.0!
