# VulkanSceneGraph Headers
For convenience the [include/vsg/all.h](all.h) header is provided that includes all headers for you.  While quick to add to your code it will likely slow compilation compared to explicitly including just the headers you need.

```C++
#include <vsg/all.h> // prefer convenience over compile speed
```

The headers that provide the library classes and definitions are organized in subdirectories with the *include/vsg/directory_name* based on the category of functionality the header provides.  All C++ classes and function definitions provided are enclosed in vsg namespace.  The vsg subdirectories/categories are:

* [include/vsg/core](core/) - Base classes & memory management

* [include/vsg/maths](maths/) - GLSL style maths classes

* [include/vsg/nodes](nodes/) - Scene graph node classes

* [include/vsg/commands](commands/) - VkCommand related scene graph classes

* [include/vsg/state](state/) - VkDescritorSet/VkDescriptor state related scene graph classes

* [include/vsg/raytracing](raytracing/) - Raytracing related scene graph classes

* [include/vsg/io](io/) - File system, stream and native file format support

* [include/vsg/ui](ui/) - User Interface Event classes

* [include/vsg/vk](vk/) - Vulkan integration classes - application level rather than scene grah lelve

* [include/vsg/traversals](traversals/) - Graph traversals

* [include/vsg/viewer](viewer/) - Viewer/Windowing classes

* [include/vsg/platform](platform/) - Platform specific Windowing/support classes

* [include/vsg/utils](utils/) - Utility classes/template functions

* [include/vsg/introspection](introspection) - introspection/reflection classes/functions
