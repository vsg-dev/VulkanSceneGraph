#pragma once

#include <string>
#include <vector>

namespace vsg
{

    using Path = std::string;

    using Paths = std::vector<Path>;

    extern Paths getEnvPaths(const char* env_var);

    extern bool fileExists(const Path& path);

    extern Path concatePaths(const Path& left, const Path& right);

    extern Path findFile(const Path& filename, const Paths& paths);

}
