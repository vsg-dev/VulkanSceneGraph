# include/vsg/io headers
The **include/vsg/io** header directory contains std::i/ostream operators, and input/output serilizations support.

## stream support
[include/vsg/io/stream.h](stream.h) provides overloads of the << and >> stream operators for std::pair<>, and the vsg::vec2, vsg::vec3, vsg::vec4 and vsg::mat4 in both their float and double variants.


## File system support
Original plan was to use C++17's filesystem support, unfortunately this is only fully supported under VisualStudio 2017 at this point in time so we've fallen back to providing a set of helper functions for checking for file existence and searching file paths.

[include/vsg/io/FileSystem.h](FileSystem.h) - provides vsg::getEnvPaths(..), fileExist(..), concactPaths(..) and findFile(..) convinience functions

Example usage:

```c++
    // read shaders
    vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");

    vsg::ref_ptr<vsg::Shader> vertexShader = vsg::Shader::read( VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert.spv", searchPaths));
    vsg::ref_ptr<vsg::Shader> fragmentShader = vsg::Shader::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag.spv", searchPaths));
```


