# 3rd Party Resources of interest

## C++ Core Guidelines etc.

* [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)
* [CppCon Resources](https://github.com/cppcon)
* [GNU Reserved-Names](https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html)

## Best Practices
* [Best Practices Criteria for Free/Libre and Open Source Software (FLOSS)](https://github.com/coreinfrastructure/best-practices-badge/blob/master/doc/criteria.md)
* [include-what-you-use tool](https://include-what-you-use.org/)
* [clang address sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
* [clang thread sanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
* [clang memory sanitizer](https://clang.llvm.org/docs/MemorySanitizer.html)

## Package managers
* [conan](https://conan.io/), [github](https://github.com/conan-io/conan)
* [hunter](https://docs.hunter.sh/en/latest/), [github](https://github.com/ruslo/hunter)
* [vcpkg](https://github.com/Microsoft/vcpkg)

## Documentation tools
* [doxygen](http://www.doxygen.org/)
* [cldoc](http://jessevdk.github.io/cldoc/)
* [DoxyPress](http://www.copperspice.com/documentation-doxypress.html)
* [Using github](http://stat545.com/bit006_github-browsability-wins.html) and [example of images, csv and pdf rendering](https://github.com/kbroman/FruitSnacks)
* [GitHub Pages](https://pages.github.com/)

## Runtime analysis tools
* [chrome://tracer](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool), [gettings started](https://google.github.io/tracing-framework/getting-started.html#installing), [JSON format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview#heading=h.5n45avt6fg8n)

## Video presentations
* [GDC 2016: High-performance, Low-Overhead Rendering with OpenGL and Vulkan](https://www.youtube.com/watch?v=PPWysKFHq9c)
* [C++Now 2017: Daniel Pfeifer “Effective CMake"]()https://www.youtube.com/watch?v=bsXLMQ6WgIk)

## Tutorials
* [Vulkan Tutorial](https://vulkan-tutorial.com/)
* [2017 Khronos UK Vulkanised  presentations](https://www.khronos.org/developers/library/2017-khronos-uk-vulkanised)
* [Multi-threading in Vulcan](https://community.arm.com/graphics/b/blog/posts/multi-threading-in-vulkan)

## Vulkan Presentations
* [Keeping-your-GPU-fed](https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/7-Keeping-your-GPU-fed.pdf)
* [Samsung Vulkan Usage Recommendations](https://developer.samsung.com/game/usage)
* [AMD Vulkan Device Memory](https://gpuopen.com/vulkan-device-memory/)
* [NVIDIA Vulkan Memory management](https://developer.nvidia.com/vulkan-memory-management)
* [Vulkan Spec](https://renderdoc.org/vkspec_chunked/index.html)
* [Vulkan Shader Resource Binding](https://developer.nvidia.com/vulkan-shader-resource-binding)
* [Lessons Learned While Building a Vulkan Material System](http://kylehalladay.com/blog/tutorial/2017/11/27/Vulkan-Material-System.html)

## Vulkan based projects
* [Pumex](https://github.com/pumexx/pumex)
* [VkHLF NVidia's C++ layer on-top of Vulkan](https://github.com/nvpro-pipeline/VkHLF)
* [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

## Maths
* [Rotors for angles](http://marctenbosch.com/quaternions/)
* [GLM](http://glm.g-truc.net) - GLSL style maths classes/functions
* [GMTL](http://ggt.sourceforge.net/html/main.html)
* [GLSL data types](https://www.khronos.org/opengl/wiki/Data_Type_GLSL)

## Image and 3D model loaders
* [Assimp](https://github.com/assimp/assimp) 3D models loading
* [GLI](http://gli.g-truc.net) Loading/manipulating images/textures
* [STB](https://github.com/nothings/stb) stb_image.h etc.
* [DevIL](http://openil.sourceforge.net/) DevIL image library, [repo](https://github.com/DentonW/DevIL)
* [FreeImage] (http://freeimage.sourceforge.net/) FreeImage image library (GPLv2/GPLv3/FreeImage License), [repo] (https://sourceforge.net/p/freeimage/code/) 

## Introspection
* [Cereal](https://github.com/USCiLab/cereal)
* [cson - C++ Simple Object Notation](https://github.com/snawaz/cson)
* [A Flexible Reflection System in C++: Part 1](http://preshing.com/20180116/a-primitive-reflection-system-in-cpp-part-1/ )
* [A Flexible Reflection System in C++: Part 2](http://preshing.com/20180124/a-flexible-reflection-system-in-cpp-part-2/)
* [Small example of use of std::make_type, std::ref etc.](http://coliru.stacked-crooked.com/a/25638f2ebc6424bf)

## 3rd party scene graphs
* Paul Martz's [JAG](https://github.com/pmartz/jag-3d/)
* Jeremy Moles' Heirograph scene graph (need reference)

## Suggestions in osg-users ML/forum post from Paweł Księżopolski
Vulkan specification is the most comprehensive source of knowledge about
the API - it's long but it is a must for any developer taking it seriously :

* [Vulkan Specification](https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html)

As for the books I recommend "Vulkan Programming Guide" written
by Graham Sellers. Sometime ago it was even available for free in
certain countries on Google Play bookstore :

* [Vulkan Programming Guide](http://www.vulkanprogrammingguide.com/)

Active forums discussing Vulkan include Khronos forums :

* [Khronos Vulkan Discussion Forum](https://forums.khronos.org/forumdisplay.php/114-Vulkan-High-Efficiency-GPU-Graphics-and-Compute)

and Vulkan subreddit :

 * [Vulkan Reddit](https://www.reddit.com/r/vulkan/)

Sascha Willems wrote a good set of Vulkan demos, that show how to
implement certain features :

* [Vulkan Demos](https://github.com/SaschaWillems/Vulkan)
* [Sascha Willems website](https://www.saschawillems.de/)
* [JHerico's Vukan examples](https://github.com/jherico/Vulkan)
* [gliterop example](https://github.com/jherico/Vulkan/tree/cpp/examples/glinterop)

Vulkan Compute examples:

* [vulkan_minimal_compute](https://github.com/Erkaman/vulkan_minimal_compute)

There's also a curated list of useful links to everything
associated with Vulkan :

* [Awesome List of Vulkan resources](https://github.com/vinjn/awesome-vulkan)

