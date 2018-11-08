# include/vsg/traversals headers
The **include/vsg/traversals** header directory contains general utility classes and functions.

## CommandLine parsing
[include/vsg/utils/CommandLine.h](CommandLine.h) provides convenience class for reading command line arguments into basic types like int, string, and compound types like vsg::vec2, std::pair<>, as well as providing a means for setting default values. 

Example usage:

```c++
int main(int argc, char** argv)
{
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);
    auto debugLayer = arguments.value(false, {"--debug","-d"});
    auto apiDumpLayer = arguments.value(false, {"--api","-a"});
    auto numFrames = arguments.value(-1, "-f");
    auto printFrameRate = arguments.value(false, "--fr");
    auto numWindows = arguments.value(1, "--num-windows");
    auto [width, height] = arguments.value(std::pair<uint32_t, uint32_t>(800, 600), {"--window", "-w"});
    if (arguments.errors()) return arguments.writeErrorMessages(std::cerr);
```

## stream support
[include/vsg/utils/stream.h](stream.h) provides overloads of the << and >> stream operators for std::pair<>, and the vsg::vec2, vsg::vec3, vsg::vec4 and vsg::mat4 in both their float and double variants.

## File system support
Original plan was to use C++17's filesystem support, unfortunately this is only fully supported under VisualStudio 2017 at this point in time so we've fallen back to providing a set of helper functions for checking for file existence and searching file paths.

[include/vsg/utils/FileSystem.h](FileSystem.h) - provides vsg::getEnvPaths(..), fileExist(..), concactPaths(..) and findFile(..) convinience functions

Example usage:

```c++
    // read shaders
    vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");

    vsg::ref_ptr<vsg::Shader> vertexShader = vsg::Shader::read( VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert.spv", searchPaths));
    vsg::ref_ptr<vsg::Shader> fragmentShader = vsg::Shader::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag.spv", searchPaths));
```
