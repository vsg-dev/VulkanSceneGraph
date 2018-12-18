## Roadmap

### 1. Exploration Phase, June-September 2018 (completed)
**Goal : Establish which technologies and broad techniques to use**

Learn and experiment with Vulkan, modern C++, and possible 3rd party dependencies.
Experimenting with different approaches to object/scene graph design and implementation. Exploration Phase Materials : 

* [Principles and Philosophy](docs/Design/DesignPrinciplesAndPhilosophy.md)
* [High Level Denis Decisions](docs/Design/HighLevelDesignDecisions.md)
* [Exploration Phase Report](docs/ExplorationPhase/VulkanSceneGraphExplorationPhaseReport.md)
* [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md)
* [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md)

### 2. Prototype Phase, October-December 2018 (present work)
**Goal : Rapid prototyping of main classes, library and test applications to establish how the scene graph API will broadly look and work.**

For a fine grained work plan see [Prototype Phase Workplan](docs/PrototypePhase/Workplan.md). The coarse grain plan follows:

* Develop as a throw away functional prototype (3rd parties should not assume any of the API is stable.)
* Create native Windowing under Windows (Win32), Linux (Xcb) and Android (done)
* Create community infrastructure - forum/mailing lists etc.. (done)
* Port to Android (initial support checked in, example being prepared)
* Scope out the best practices that will achieve rapid and robust software development once work on the final VSG starts:
    * Static code analysis (cppcheck added 19.10.18)
    * Dynamic analysis, such as LLVM sanitizers) and valgrind
    * Follow [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
    * Follow [FOSS Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md)
* Build add-on libraries, as separate projects, to provide additional features:
    * 3rd party image loading
    * 3rd party 3D model loading (including FBX)
    * Basic OpenSeneGraph integration via osg2vsg to help import image and model data into VSG.
    * [GLSLang](https://github.com/KhronosGroup/glslang) shader compilation
* Implement basic culling and state management
* Create suite of test applications that can aid in feature development and testing
* Prototyping to inform plans for Core Development Phase



### 3. Core Development Phase, January-Summer 2019
**Goal: Implement the final class interfaces and implementation**

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
