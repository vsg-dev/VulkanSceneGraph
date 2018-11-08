# Vulkan/VkSceneGraph Headers
For convenience the [include/vsg/all.h](all.h) is provided that includes all headers for you.  While quick to add to your code it will likely slow you compile compared to explicitly including just the headers you need.

```C++
#include <vsg/all.h> // prefer convenience over compile speed
```

The headers that provide the library classes and definitions are organized in subdirectories with the include/vsg directory based on the category of functionality the header provides.  All C++ classes and function definitions provided are enclosed in vsg namespace.  The vsg subdirectories/categories are:

### [core](core/) - Base classes & memory management

### [maths](maths/) - GLSL style maths classes

### [nodes](nodes/) - Graph node classes 

### [traversals](traversals/) - Graph traversals

### [vk](vk/) - Vulkan integration classes

### [viewer](viewer/) - Viewer/Windowing classes

### [utils](utils/) - Utility classes/template functions

### [introspection](introspection) - introspection/reflection classes/functions