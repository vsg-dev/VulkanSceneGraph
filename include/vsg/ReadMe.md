# Vulkan/VkSceneGraph Headers
For convenience the [include/vsg/all.h](all.h) header is provided that includes all headers for you.  While quick to add to your code it will likely slow you compile compared to explicitly including just the headers you need.

```C++
#include <vsg/all.h> // prefer convenience over compile speed
```

The headers that provide the library classes and definitions are organized in subdirectories with the *include/vsg/directory_name* based on the category of functionality the header provides.  All C++ classes and function definitions provided are enclosed in vsg namespace.  The vsg subdirectories/categories are:

### [include/vsg/core](core/) - Base classes & memory management

### [include/vsg/maths](maths/) - GLSL style maths classes

### [include/vsg/nodes](nodes/) - Graph node classes 

### [include/vsg/traversals](traversals/) - Graph traversals

### [include/vsg/vk](vk/) - Vulkan integration classes

### [include/vsg/viewer](viewer/) - Viewer/Windowing classes

### [include/vsg/utils](utils/) - Utility classes/template functions

### [include/vsg/introspection](introspection) - introspection/reflection classes/functions