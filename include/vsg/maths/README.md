# include/vsg/maths headers
The **include/vsg/maths** header directory contains the vector, matrix classes and maths functions. The interface and conventions are kept the same as GLSL so C++ usage can be kept consistent with shaders usage.

The vector and maths classes are simple types, do not subclass from vsg::Object, so should be treated like ints, floats etc. The memory storage used aligns with the types expected by Vulkan so can be used directly for uniform, vertex and image data.

## Vector classes
* [include/vsg/maths/vec2.h](vec2.h) - template class for 2d vectors, provides vsg::vec2 (float), vsg::dvec2 (double) versions.
* [include/vsg/maths/vec3.h](vec3.h) - template class for 3d vectors, provides vsg::vec3 (float), vsg::dvec3 (double) versions.
* [include/vsg/maths/vec4.h](vec4.h) - template class for 4d vectors, provides vsg::vec4 (float), vsg::dvec4 (double) versions.

## Matrix classes
* [include/vsg/maths/mat4.h](mat4.h) - template class for 4x4 matrix, providing vsg::mat4 (float) and vsg::dmat4 (double) versions.

## Matrix/Vector support functions
* [include/vsg/maths/transform.h](transform.h) - provides a range of convenience functions for creation of matrices and operations on matrices and vector.
