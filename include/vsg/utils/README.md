# include/vsg/utils headers
The **include/vsg/utils** header directory contains general utility classes and functions.

## CommandLine parsing
[include/vsg/utils/CommandLine.h](CommandLine.h) provides convenience class for reading command line arguments into basic types like int, string, and compound types like vsg::vec2, std::pair<>, as well as providing a means for setting default values.

Example usage:

```c++
int main(int argc, char** argv)
{
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);
    auto debugLayer = arguments.read({"--debug","-d"});
    auto apiDumpLayer = arguments.read({"--api","-a"});
    auto printFrameRate = arguments.read("--fr");
    auto numFrames = arguments.value(-1, "-f");
    auto numWindows = arguments.value(1, "--num-windows");
    auto [width, height] = arguments.value(std::pair<uint32_t, uint32_t>(800, 600), {"--window", "-w"});
    if (arguments.errors()) return arguments.writeErrorMessages(std::cerr);
    ...
}
```
