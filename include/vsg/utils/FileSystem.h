#pragma once

#include <string>
#include <vector>

#include <vsg/core/Export.h>

namespace vsg
{

    using Path = std::string;

    using Paths = std::vector<Path>;

    extern VSG_EXPORT Paths getEnvPaths(const char* env_var);

    extern VSG_EXPORT bool fileExists(const Path& path);

    extern VSG_EXPORT Path concatePaths(const Path& left, const Path& right);

    extern VSG_EXPORT Path findFile(const Path& filename, const Paths& paths);

}
