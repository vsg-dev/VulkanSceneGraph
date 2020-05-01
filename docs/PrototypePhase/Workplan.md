# Prototype Phase Work Plan
The aim of the Prototype Phase, October-December 2018, is to fill out prototypes for main elements of the core VSG scene graph library, add-on libraries and test programs. These prototypes will help solidify the choices in supporting technologies used and the design and implementation approaches used.

## General project infrastructure

We need to flesh out the following high level project infrastructure:

- [x] Layout used in core, add-on and supporting applications/examples.
	- Follow [FOSS Best Practices](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md)
- [x] Conventions (naming, coding style etc.) used in core, add-on and supporting applications/examples.
    - [x] Follow [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [x] Website(s) - what requires dedicated websites, vs embedded in git-hub repositories.
	- [x] Initial approach to see how rich the experience can be with just README.md etc.
- [x] Developer/Community discussion forum(s)
	- [x] Create googlegroup [VulkanSceneGraph Developer Discussion Group](https://groups.google.com/forum/#!forum/vsg-users)
- [ ] 3rd party tools used in development and testing
	- [x] Static code analysis (cppcheck added 19.10.18)
    - [ ] Dynamic analysis such as LLVM sanitizers and valgrind

## Core Scene Graph development
Core scene graph work will be primarily tackled by Robert Osfield.

- [x] Support for object serialization and reading/writing to native ascii/binary format.
- [x] Creation of vsg::Array2d (completed), vsg::Array3D (completed) classes to mirror the existing [vsg::Array](../../include/vsg/core/Array.h) class to enable more streamlined support for texture data.
- [ ] Create a unified memory allocator interface to enable custom memory allocation for both scene graph objects and Vulkan objects.  Need to bring together the current [vsg::Allocator](../../include/vsg/core/Allocator.h) and [VkAllocationCallbacks](../../include/vsg/vk/AllocationCallback.h)
- [ ] Flesh out more [RecordTraversal](../../include/traversals/RecordTraversal.h) class that traverses the Command Graph recording commands into a CommandBuffer, integrating culling.
- [ ] Investigate unifying the [vsg::QuadGroup](../../include/nodes/QuadGroup.h) and [vsg::Group](../../include/nodes/Group.h) classes using a small size array optimization approach. Currently vsg::QuadGroup with is fixed size children array is faster than vsg::Group for creation and traversal, but is less flexible. Can we avoid the user space complexity of having two classes using small size optimizations?
- [ ] Restructure the scene graph level Vulkan object creation to use on-demand Vulkan object creation to enable handling of multiple logical devices and scene graph creation prior to viewer/logical device setup.
- [ ] Restructure the viewer/window level Vulkan object creation so that it uses a more standard MyClass::create(..) API and returns a vsg::ref_ptr<MyClass> rather than a vsg::Result<MyClass>. Use exceptions to handle errors.

## Cross platform support
The initial development work has been done under Linux, with support for additional platform to be tackled in the prototype phase. Cross platform work to be led by Thomas Hogarth.

- [x] Initial Windows support was added in October, this will be refined to provide as straight forward developer experience as we can achieve.
- [x] Development of platform specific Windowing support to replace the dependency on GLFW (done.)
	- [x] Win32_Window native windowing class for Windows
	- [x] Xcb_Window native Windowing for Unices
- [x] Port to Android : completed with vsgExample/Android example illustrating how to create an Android application
- [x] Port to macOS near completion (just key events left to resolve) with  native windowing provided by MacOS_Window.mm 

## Add-on library development
Add-on libraries will provide image and 3d model loaders, and integration with 3rd party software.

- [ ] [osg2vsg](https://github.com/vsg-dev/osg2vsg) OpenSceneGraph/VSG integration support library:
	- [x] Convert existing osg::Image loading/vulkan object creation to use vsg::Array2D(completed)/3D
	- [ ] Basic support for converting osg::Node scene graph objects to vsg::Node equivalents

- [ ] vsg*Image - possible integration of the 3rd party image readers/writers
- [ ] vsg*Model - possible integration of the 3rd party model readers/writers
- [ ] vsgGLSLang - possible integration with the [GLSLslang](https://github.com/KhronosGroup/glslang) library for reading GLSL shaders and converting to SPIRV shaders compatible with Vulkan/VSG.

## Example/Testbed development
All the software developed above needs testing, so we need to continue to expand the list of test applications that can test both the API usage and runtime behaviour/performance. Testing software is the primary focus during the **Prototype Phase** so applications developed during just as examples will not be attempted. The test programs can still serve as examples for others to learn from. Two main places for testbed development will be:
- [ ] [vsgExamples](https://github.com/vsg-dev/vsgExamples) - a set of test programs that will later evolve into our example set. Unit tests will likely need to be spawned off this project, possibly integrated into core VSG repository.
- [ ] [vsgFramework](https://github.com/vsg-dev/vsgFramework) - an experiment with using CMake to find external dependencies and if they aren't available fallback to using  [ExternalProject_Add()](https://cmake.org/cmake/help/latest/module/ExternalProject.html) to check out and build 3rd party dependencies.
