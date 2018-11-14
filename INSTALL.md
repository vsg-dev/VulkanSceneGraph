## Prerequisites
* C++17 compliant compiler i.e. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.7 or later.
* [GLFW](https://www.glfw.org)  3.3 or later.  The plan is to implement native Windowing support so this dependency will
 later be removed.

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against older versions. If you find success with older versions let us know and we can update the version info.

---

## Quick guide to build and install the VSG for Unix from the command line:

Command line instructions for default build of static library (.a/.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake .
    make -j 8
    make install

Command line instructions for building shared library (.so/.lib + .dll) in out of source:

    git clone https://github.com/vsg-dev/VulkanSceneGraphPrototype.git
    mkdir vsg-shared-build
    cd vsg-shared-build
    cmake ../VulkanSceneGraphPrototype -DBUILD_SHARED_LIBS=ON
    make -j 8
    make install

---

## Quick guide to build and install the VSG for Windows using Visual Studio 2017

Command line instructions for default build of static library (.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake . -G "Visual Studio 15 2017 Win64"
    
After running cmake open the generated VSG.sln file and build the All target. Once built you can run the install target. If you are using the default cmake install path (in Program Files folder), ensure you have started Visual Studio as administrator otherwise the install will fail.

---

## Quick guide to build and install the VSG for macOS using Xcode 9

Command line instructions for default build of static library (.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake . -G "Xcode"
    
After running cmake open the generated VSG.xcodeproj file and build the All target. Once built you can run the install target. Please note that for release builds you currently need to use the Archive option in xcode. This will rebuild everytime so you can just select the install target and run Archive which will also build the All target.

---

## Available build targets

Once you have generated the build system using *cmake* as above, you can list the available make options using:

    make help

This lists the options:

    ... all (the default if no target is provided)
	... clean
	... depend
	... install/strip
	... install/local
	... rebuild_cache
	... clobber
	... install
	... docs
	... uninstall
	... build_all_h
	... list_install_components
	... cppcheck
	... clang-format
	... edit_cache
	... vsg

Most of these are standard options which you can look up in CMake and make documentation, the following are ones we've added so require explanation:

    # remove all files not part of the git repository - including all temporary CMake and build files.
    make clobber

	# run cppcheck on headers & source to generate a static analysis
    make cppcheck

    # run clang-format on headers & source to format to style specified by .clang-format specification
    make clang-format

    # generate the include/vsg/all.h from all the files that match include/vsg/*/*.h
    make build_all_h

---

## Using the VSG within your own projects

The project is currently a prototype that is undergoing continuous development so it isn't recommend to use as base for long term software development. At this point it's available for developers who want to test the bleeding edge and provide feedback on it's fitness for purpose. Following instructions assume your project uses CMake, which at this early stage in the project is the recommended route when using the VSG.

To assist with setting up software to work with the VSG when you install the library a CMake package configuration file will be installed in the lib/cmake/vsg directory. Within your CMake CMakeLists.txt script to find the VSG related dependencies you'll need to add:

	find_package(vsg)

To select C++17 compilation you'll need:

	set_property(TARGET mytargetname PROPERTY CXX_STANDARD 17)

To link your lib/application to required dependencies you'll need:

	target_link_libraries(mytargetname vsg::vsg)

This will tell CMake to set up all the appropriate include paths, libs and any definitions (such as the VSG_SHARED_LIBRARY #define that is required under Windows with shared library builds to select the correct declspec().)

For example, a bare minimum CMakeLists.txt file to compile a single file application would be:

	cmake_minimum_required(VERSION 3.7)
	find_package(vsg REQUIRED)
	add_executable(myapp "myapp.cpp")
	set_property(TARGET myapp PROPERTY CXX_STANDARD 17)
	target_link_libraries(myapp vsg::vsg)

---
	
## Detailed instructions for setting up your environment and building for Microsoft Windows

VSG application currently have two main dependancies, the Vulkan SDK itself and for now GLFW for window creation. LunarG provides a convient installer for the Vulkan SDK and runtime on Windows.

[Vulkan Downloads] (https://vulkan.lunarg.com/sdk/home#windows)

From there download and install the Vulkan SDK (1.1 or later) and the Vulkan runtime. Once installed we need to let CMake know where to find the Vulkan SDK. Both VSG and GLFW use the VULKAN_SDK environment variable to find the Vulkan SDK so go ahead and add it.

	VULKAN_SDK = C:\VulkanSDK\1.1.85.0

Next we need to download build and install GLFW with Vulkan support. GLFW does provide prebuilt binaries for Windows. However they do not ship the CMake.config files required for CMake to find the GLFW headers and lib.

    git clone https://github.com/glfw/glfw.git
    cd ./glfw
    cmake . -G "Visual Studio 15 2017 Win64"

Once CMake is finished open the generated GLFW.sln file and build the Install target for release. Remember to open Visual Studio as Administrator if installing to the Program Files folder, the default. Finally CMake needs to know where GLFW is installed on your system. As it's using CMake.config files we use the CMAKE_PREFIX_PATH environment variable to inform CMake of the location of our installed libraries. So go ahead and add GLFW to this list. Example below.

    CMAKE_PREFIX_PATH = C:\Program Files\GLFW;C:\Program Files\VSG

You can see in the example that we also have VSG in the list, this will be required later once VSG is built and installed and we want to utilise it with other CMake based projects.

So now we have the Vulkan SDK and GLFW installed and findable by CMake so we can go ahead and build VSG. Below are simple instructions for downloading the VSG source code, generating a Visual Studio project using CMake and finally building and installing VSG onto your system.

    git clone https://github.com/vsg-dev/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake . -G "Visual Studio 15 2017 Win64"
    
After running CMake open the generated VSG.sln file and build the All target. Once built you can run the install target. If you are using the default CMake install path (in Program Files folder), ensure you have started Visual Studio as administrator otherwise the install will fail.
