VulkanSceneGraphPrototype (VSG) is a prototype for a modern scene graph library built upon Vulkan graphics/compute API.  The software is written in C++17, and follows the [CppCoreGuidlines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).

## Prerequisites
* C++17 complient compiler i.e. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.5 or later.
* [GLFW](https://www.glfw.org)  3.3 or later.  The plan is to implement native Windows so this dependency will
 later be removed.

It may be possible to build the VSG with older versions of the above dependencies, the above combination are known to work, if you find success with older versions let us know and we can related the version info.

## Building the VSG

Command line instructions for building static library (default .a/.lib) in source:

    git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake .
    make -j 8
    make install

Command line instructions for building shared library (.so/.lib + .dll) in out of source:

> git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git

> mkdir vsg-sharead-build

> cd vsg-shared-build

> cmake ../VulkanSceneGraphPrototype -DBUILD_SHARED_LIBS=ON

> make -j 8

> make install

## Documents
* [Areas of Interest](docs/AreasOfInterest.md)
* [3rd Party Resources](docs/3rdPartyResources.md)
