# Areas of Interest during Exploration Phase

## General

* Selection of Open Source License
* Project websites
* Community mailing list/forum
* Social media
* Testing frameworks/approach/projects - [SceneGraphTestBed](https://github.com/openscenegraph/SceneGraphTestBed)
* Build tools
  * [CMake](https://cmake.org/)
  * [xmake](https://xmake.io/#/)

## Vulkan

* Setup and Configuration
* State and geometry data
  * Creation of buffers
  * Uniforms
  * Textures
  * Arrays
  * Primitives
* Shaders
  * tools for offline and runtime .glsl -> SPIR-V
  * runtime shader compilation options
* Presentation of graphics
* Threading
* Synchronization

## C++
* Features and availability of C++ 11, 14 and 17
* Threading
* Containers
* Algorithms
* Memory management
* Templates
* Lambda
* Style Guide

## Core class
* Memory
  * ref_ptr
  * Object
  * Backbone data - local data used during standard traversals/operations
  * Extra - data used rarely : parents, observers, user data
* Introspection
  * Wrappers
  * Serialization
  * I/O
* Threading
  * C++ now has threading but no Affinity
  * Thread pools

## Scene Graph
* Scope out the minimal set of Node classes required
  * NullNode to avoiding the need for if (!node) doSomething
  * Fixed size vs variable size Containers in Groups etc.
  * Level of Detail
  * CullNode (shift bounding volumes from all nodes to specialized node?)
  * MaskNode (shift node mask checks from all nodes to specialized node?)
  *Cameras
* Traversal
  * Visitor Pattern
  * Possibilities for inlining vs virtual functions
  * RenderTraversal
  * ComputeTraversal
  * UpdateTraversal
  * EventTraversal
  * Multi-pass and Multi-stage rendering control
* State
  * Buffers
  * Uniforms
  * Textures
  * Arrays
  * Primitives
  * Shaders
  * Shader composition
* Threading
  * Traversals
  * Database paging

## Viewer
* Window creation
* Vulkan initialization and synchronization
* Presentation of graphics, swap management
* Threading
* Single view vs multi view

## Interoperability
* Converting scene graph data from OpenSceneGraph data to leverages loaders
* Rendering Vulkan graphics within a OpenGL graphics context

