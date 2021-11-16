# src/vsg source directories

The implementations provided by the src/vsg directories mirror the structure of the include/vsg header directory structure.

## Lower level classes that provide basic glue of the scene graph
* [src/vsg/core](core) - Base classes that provide reference counting, arrays, visitors.
* [src/vsg/maths](maths) - GLSL style maths classes.
* [src/vsg/io](io) - File system, stream and native file format support
* [src/vsg/threading](threading) - Threading class that build upon std::thread.
* [src/vsg/utils](utils) - Utility functions/classes.

## Vulkan integration classes and associated scene graph nodes
* [src/vsg/vk](vk) - classes that provide wrappers to high level Vulkan objects, providing robust resource management and convenient C++ style setup.
* [src/vsg/state](state) - scene graph level classes that provide wrappers Vulkan object with setting Vulkan state such as Pipelines, Uniforms and Textures.
* [src/vsg/commands](commands) - scene graph level classes that provide wrappers to vkCmd* Vulkan API calls.
* [src/vsg/rtx](rtx) - scene graph level classes that provide wrappers to Vulkan RTX ray tracing and mesh shader extensions.

## Scene graphs nodes
* [src/vsg/nodes](nodes) - scene graph node classes that provide the internal structure to the scene graph.
* [src/vsg/text](text) - scene graph text classes that provide convenient support for high quality text.

## Application level
* [src/vsg/traversals](traversals) - traversal implementations such as CompileTraversal, & RecordTraversal.
* [src/vsg/platform](platform) - platform specific implementations of Windowing.
* [src/vsg/viewer](viewer) - Viewer and Application classes
* [src/vsg/introspection](introspection) - initial exploration of introspection.
