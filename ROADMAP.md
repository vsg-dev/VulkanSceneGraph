## Roadmap

### 1. Exploration Phase, June-September 2018 (completed)
**Goal : Establish which technologies and board techniques to use**

Learn and experiment with Vulkan, modern C++, and possible 3rd party dependencies.
Experimenting with different approaches to object/scene graph design and implementation. Exploration Phase Materials : 

* [Areas of Interest](docs/ExplorationPhase/AreasOfInterest.md)
* [3rd Party Resources](docs/ExplorationPhase/3rdPartyResources.md)

### 2. Prototype Phase, October-December 2018 (present work)
**Goal : Rapid prototyping of main classes, library and test applications to establish how the scene graph API will broadly look and work.**

* Develop as a throw away functional prototype (3rd parties should not assume any of the API is stable.)
* Scope out the best practices that will achieve rapid and robust software development once work on the final VSG starts:
    * Static code anaysis (cppcheck has been added.)
    * Dynamic analysis (such as LLVM sanitizers)
    * Follow [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
    * Follow [Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md)
* Build addon libraries, as sparate projects, to provide additional features:
    * 3rd party image loading
    * 3rd party model loading
    * [GLSLang](https://github.com/KhronosGroup/glslang) shader compilation
* Create suite of test applications that can aid in feature development and testing
* Port to Android.
* Create community infrastrucure - forum/mailng lists etc.
* Prototyping to inform plans for Core Development Phase

### 3. Core Development Phase, January-Summer 2019
**Goal: Implement the final class interfaces and implementation**

Using the protyping work as a guide implement the final scene graph library with the aim of creatig a solid interface and implementation.

* Development of final VSG Library.
* Development of add on libraries.
* Development of test suite of programs and data.
* Port to OSX and iOS.

### 4. Release Phase,  Fall 2019 onwards
**Goal: Test scene graph library against real-world applications and shake down the API and implementation for it's first stable release.**

* Refinement of API and implementation.
* Build relationships with appliction developers and involve them in testing.
* Create tutorial and example programs to illustrate how to use VSG.
* Test, debug, refine and release 1.0.0!
