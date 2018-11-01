## Prerequisites
* C++17 compliant compiler i.e. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.
* [CMake](https://www.cmake.org) 3.5 or later.
* [GLFW](https://www.glfw.org)  3.3 or later.  The plan is to implement native Windowing support so this dependency will
 later be removed.

The above dependency versions are known to work so they've been set as the current minimum, it may be possible to build against oldeer versions.  If you find success with older versions let us know and we can related the version info.

---

## Quick guide to build and install the VSG from the command line:

Command line instructions for default build of static library (.a/.lib) in source:

    git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git
    cd VulkanSceneGraphPrototype
    cmake .
    make -j 8
    make install

Command line instructions for building shared library (.so/.lib + .dll) in out of source:

    git clone https://github.com/robertosfield/VulkanSceneGraphPrototype.git
    mkdir vsg-shared-build
    cd vsg-shared-build
    cmake ../VulkanSceneGraphPrototype -DBUILD_SHARED_LIBS=ON
    make -j 8
    make install

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
	... build_all_h
	... list_install_components
	... cppcheck
	... clang-format
	... edit_cache
	... vsg

Most of these are standard options which you can look up in CMake and make documentation, the following are ones we've added so require explanation:

    make clobber      # remove all files not part of the git repository - including all temporary CMake and build files.
    make cppcheck     # run cppcheck on headers & source to generate a static analysis
    make clang-format # run clang-format on headers & source to format to style specified by .clang-format specification
    make build_all_h  # generate the include/vsg/all.h from all the files that match include/vsg/*/*.h

---

## Using the VSG within your own projects

The project is currently a prototype that is undergoing continuous development so it isn't recommend to use as base for long term software development. At this point it's available for developers who want to test the bleeding edge and provide feedback on it's fitness for purpose.

To assist with setting up software to work with the VSG we provided [FindVSG.cmake](CMakeModules/FindVSG.cmake), [FindGLFW.cmake](CMakeModules/FindGLFW.cmake) and [FindVulkan.cmake](CMakeModules/FindVulkan.cmake) within the [VullkanSceneGraphPrototype/CMakeModules](CMakeModules) directory.  Feel free to copy these into your own project.

Within your CMake CMakeLists.txt script to find the VSG related dependencies you'll need to add:

   find_package(VSG)
   find_package(GLFW)
   find_package(Vulkan)

To select C++17 compilation you'll need:

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

To link your lib/application to required dependnecies you'll need:

    target_link_libraries(mytargetname VSG::VSG GLFW::GLFW Vulkan::Vulkan)

This will tell CMAke to set up all the appropriate include paths, libs and any definitions (such as the VSG_SHARED_LIBRARY #define that is required under Windows with shared library builds to select the correct declspec().)

