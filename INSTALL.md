## Build and Install instructions index
* [Prerequisites](#prerequisites) - list of project external dependencies
* Unix
	* [Quick Unix instructions](#quick-build-instructions-for-unix-from-the-command-line)
* Windows
	* [Quick Windows instructions](#quick-build-instructions-for-windows-using-visual-studio-2017)
	* [Detailed Windows instructions](#detailed-instructions-for-setting-up-your-environment-and-building-for-microsoft-windows)
* Android
	* [Quick Android instructions](#quick-build-instructions-for-android)
	* [Detailed Android instructions](#detailed-instructions-for-setting-up-your-environment-and-building-for-android)
* macOS
	* [Quick macOS instructions](#quick-build-instructions-for-macOS-using-xcode-9)
	* [Detailed macOS instructions](#detailed-instructions-for-setting-up-your-environment-and-building-for-macos)
* [Build targets](#available-build-targets)
* [Using the VSG within your own projects](#using-the-vsg-within-your-own-projects)

---

## Prerequisites

* C++17 compliant compiler i.e. g++ 7.3 or later, Clang 6.0 or later, Visual Studio S2017 or later.
* [Vulkan](https://vulkan.lunarg.com/) 1.1 or later.  You can use Vulkan (libs and headers) installed from repositoies or using VulkanSDK.
* [CMake](https://www.cmake.org) 3.7 or later.

## Optional dependenices

* [glslang & SPIRV-Tools](https://github.com/KhronosGroup/glslang) are required when built-in GLSL -> SPIR-V (required by Vulkan) compilation is needed, such as when you need the VulkanSceneGraphs shader composition and compilation capailities. SPIRV-Tools nowadays is packaged separately on common linux distributions, but can be build as part of glslang. VulkanSDK provides glslang. Unless you know you don't require them for your application we recommend building the VulkanSceneGraph with glslang and SPIRV-Tools.

---

## Quick build instructions for Unix from the command line

### Gentoo dependencies
1. essential dependencies:
	emerge dev-util/vulkan-tools

2. optional dependencies:
	emerge dev-util/glslang dev-util/spirv-tools 

### Build
Command line instructions for default build of static library (.a/.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    cmake .
    make -j 8
    make install

Command line instructions for building shared library (.so/.lib + .dll) out of source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    mkdir vsg-shared-build
    cd vsg-shared-build
    cmake ../VulkanSceneGraph -DBUILD_SHARED_LIBS=ON
    make -j 8
    make install


---

## Quick build instructions for Windows using Visual Studio 2017

Command line instructions for default build of static library (.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    cmake . -G "Visual Studio 15 2017 Win64"

After running cmake open the generated VSG.sln file and build the All target. Once built you can run the install target. If you are using the default cmake install path (in Program Files folder), ensure you have started Visual Studio as administrator otherwise the install will fail.

More detailed Windows platform instructions can be found [below](#detailed-instructions-for-setting-up-your-environment-and-building-for-microsoft-windows).

---

## Quick build instructions for Android

Requires Android NDK 18 and CMake 3.13 (lower CMake versions may work but have not been tested).

	cmake ./ \
	-DCMAKE_BUILD_TYPE="Debug" \
	-DCMAKE_SYSTEM_NAME="Android" \
	-DCMAKE_SYSTEM_VERSION=24 \
	-DCMAKE_ANDROID_STL_TYPE="c++_static" \
	-DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
	-DCMAKE_ANDROID_NDK=/location/of/Android/sdk/ndk-bundle \
	-DCMAKE_INSTALL_PREFIX=/usr/local/android

	make -j 8
	make install

More detailed Android platform instructions can be found [below](#detailed-instructions-for-setting-up-your-environment-and-building-for-android).

---

## Quick build instructions for macOS using Xcode 9

Command line instructions for default build of static library (.lib) in source:

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    cmake . -G "Xcode"

After running cmake open the generated VSG.xcodeproj file and build the All target. Once built you can run the install target. Please note that for release builds you currently need to use the Archive option in xcode. This will rebuild every time so you can just select the install target and run Archive which will also build the All target.

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

### Using VSG provided cmake macros within your own projects

The build system provides macros that create specific cmake targets to use in their project. Examples include: Setup of common cmake variables, Formatting source code, performing static code analysis, creating API documentation, cleaning up source directories, and removing installed files. Documentation of the available macros (the public ones starting with ```vsg_```) are at https://github.com/vsg-dev/VulkanSceneGraph/blob/master/cmake/vsgMacros.cmake.

For example, a bare minimum CMakeLists.txt file adding the mentioned cmake target would be:

	cmake_minimum_required(VERSION 3.7)
	find_package(vsg REQUIRED)

	vsg_setup_dir_vars()
	vsg_setup_build_vars()

	vsg_add_target_clang_format(
	    FILES
	        ${PROJECT_SOURCE_DIR}/include/*/*.h
	        ${PROJECT_SOURCE_DIR}/src/*/*.cpp
	)
	vsg_add_target_cppcheck(
	    FILES
	        ${PROJECT_SOURCE_DIR}/include/*/*.h
	        ${PROJECT_SOURCE_DIR}/src/*/*.cpp
	)
	vsg_add_target_clobber()
	vsg_add_target_docs(
	    FILES
	        ${PROJECT_SOURCE_DIR}/include/*/*.h
	)
	vsg_add_target_uninstall()

	add_executable(myapp "myapp.cpp")
	set_property(TARGET myapp PROPERTY CXX_STANDARD 17)
	target_link_libraries(myapp vsg::vsg)

### Using VSG provided cmake macro to generate cmake support files

Projects that install a library must generate some cmake-related files so that the library can be found by ```find_package()```. To simplify the generation of these files, the cmake macro ```vsg_add_cmake_support_files()``` has been added. 

In addition to calling the macro, it requires a template for creating the xxxConfig.cmake file, as given in the following example:

	src/xxx/
	      CMakeLists.txt
	      xxxConfig.cmake.in

In the file ``CMakeLists.txt`` the call then looks like this:

	    vsg_add_cmake_support_files(
	        CONFIG_TEMPLATE xxxConfig.cmake.in
	    )

Hints for the structure of the template file can be found at https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-a-package-configuration-file.

---

## Detailed instructions for setting up your environment and building for Microsoft Windows

While not the only route to installing Vulkan libs an headers on Windows the most common approach is to use the Vulkan SDK. LunarG provides a convenient installer for the Vulkan SDK and runtime on Windows.

[Vulkan Downloads](https://vulkan.lunarg.com/sdk/home#windows)

From there download and install the Vulkan SDK (1.2.162.0 or later) and the Vulkan runtime. Once installed we need to let CMake know where to find the Vulkan SDK. The Vulkan installer should add both ```VK_SDK_PATH``` and ```VULKAN_SDK```. If either of those are not present - add them.

    VK_SDK_PATH = C:\VulkanSDK\1.2.162.0
    VULKAN_SDK = C:\VulkanSDK\1.2.162.0

So now we have the Vulkan SDK installed and findable by CMake so we can go ahead and build VSG. Below are simple instructions for downloading the VSG source code, generating a Visual Studio project using CMake and finally building and installing VSG onto your system.

    git clone https://github.com/vsg-dev/VulkanSceneGraph.git
    cd VulkanSceneGraph
    mkdir build & cd build
    cmake .. -G "Visual Studio 15 2017 Win64"
    cmake .. -G "Visual Studio 16 2019" -A x64
    cmake .. -G "Visual Studio 19 2022" -A x64

After running CMake open the generated VSG.sln file and build the All target. Once built you can run the install target. If you are using the default CMake install path (in Program Files folder), ensure you have started Visual Studio as administrator otherwise the install will fail.

It's recommended at this point that you add the VSG install path to you CMAKE_PREFIX_PATH, this will allow other CMake projects, like the vsgExamples project to find your VSG installation. CMAKE_PREFIX_PATH can be set as an environment variable on you system.

    CMAKE_PREFIX_PATH = C:\Program Files\VSG

---

## Detailed instructions for setting up your environment and building for Android

This guide is to build VSG for Android, these steps have been completed on macOS but should be almost identical on Linux and similar on Windows. Inorder to build VSG for Android you'll need the following installed on your machine.

	Android NDK 18
	CMake 3.13

The easiest way to get the Android NDK installed is via Android Studio. Follow the link below to download and install it for your OS.

[Android Studio](https://developer.android.com/studio/)

If you got to the 'SDK Manager' ensure you have at least Android API level 24 installed, then go to the 'SDK Tools' tab and check the 'NDK' option. Once done click apply and Android Studio should download and install these components for you.

If you already have Android Studio and or the NDK installed. Still go to the 'SDK Manager' and see if you need to update your NDK to version 18.

Take note of the 'Android SDK Location' as you'll need it when running CMake to generate our Android make files.

So now we have the Android NDK installed lets go ahead and fetch the VSG source then use CMake to generate the make files.

	git clone https://github.com/vsg-dev/VulkanSceneGraph.git
	cd VulkanSceneGraph
	cmake ./ \
	-DCMAKE_BUILD_TYPE="Debug" \
	-DCMAKE_SYSTEM_NAME="Android" \
	-DCMAKE_SYSTEM_VERSION=24 \
	-DCMAKE_ANDROID_STL_TYPE="c++_static" \
	-DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
	-DCMAKE_ANDROID_NDK=/location/of/Android/sdk/ndk-bundle \
	-DCMAKE_INSTALL_PREFIX=/usr/local/android

Make sure you change the -DCMAKE_ANDROID_NDK path to the path of your NDK, typically this is the 'Android SDK Location'/ndk-bundle. Also note the -DCMAKE_INSTALL_PREFIX. This is where the VSG library and header will be installed. It's useful to change this from the default to separate your Android version from your native OS version. Depending where you put it you may need to manually create the top level folder first depending on permissions.

Now we've generated the make files we can simply run

	make -j 8
	make install

That's it, you've built VSG for Android and installed the required headers and library files onto your machine ready to use with your project or the Android vsgExamples.


## Detailed instructions for setting up your environment and building for macOS

macOS does not natively support Vulkan. However the excellent MoltenVK library has been developed which translates Vulkan calls into the Metal equivalents allowing you to run Vulkan applications on macOS and iOS. This can be downloaded from the LunarG website and has been packaged in a way it's extremely similar to the other platform sdks.

[Vulkan Downloads](https://vulkan.lunarg.com/sdk/home#mac)

Download the sdk and unpack it. There's no form of installer but you're able to place the root sdk folder anywhere on your machine. The sdk contains detailed instuctions on how to setup MoltenVK to work on your machine as well has how to redistribute your application contained in the /Documentation/getting_started_macos.md file.

As with other platforms we need the VULKAN_SDK variable which should point to your downloaded sdk folder. Specifically the macOS subfolder of the sdk. This is needed so CMake can find the sdk. Unique to macOS we also need to set environment variables pointing a few files within the sdk. Again the getting started document in the sdk has detailed information relating to these. A quick cheat sheet is provided here, if you use a .bash_profile file in your user folder you can add the following.

	export VULKAN_SDK="/path/to/your/vulkansdk/macOS"
	export VK_LAYER_PATH="$VULKAN_SDK/etc/vulkan/explicit_layer.d"
	export VK_ICD_FILENAMES="$VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json"
	export PATH="$VULKAN_SDK:$VULKAN_SDK/bin:$PATH"
	export DYLD_LIBRARY_PATH="$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH"

At this point MoltenVK application should be able to run on your machine, as a quick test it's worth running vulkaninfo executable contained within vulkansdk/macOS/bin. If this runs and doesn't produce any errors it's a good indication the sdk is setup correctly.

So now we're ready to build VSG. With the SDK installed this is very similar to other platforms. You can simple run the following commands to clone the source and use CMake to generate and Xcode project.

	git clone https://github.com/vsg-dev/VulkanSceneGraph.git
	cd VulkanSceneGraph
	cmake . -G "Xcode"
	
Once CMake has finished you can open the generated Xcode project and build the 'install' target. This will build VSG and install the headers and generated library onto your machine.

Again, as with other platforms it's useful to now set your CMAKE_PREFIX_PATH to point to the VSG library we have just installed. If you've installed to the default location you can add the following to you .bash_profile file.

	export CMAKE_PREFIX_PATH="/usr/local/lib/cmake/vsg"
	
That's it, we've installed the MoltenVK sdk, built VSG and prepared are machine so other CMake projects can find and use the VSG library.

**Important Note!**

Xcode typically ignores the system environment variables, so when running a VSG application from within Xcode you may run into issues. One solution is to add the environment variables to to the run scheme. This can be done by going to 'Product>Scheme>Edit Scheme>Arguments. Then added the above mentioned environment variables.




