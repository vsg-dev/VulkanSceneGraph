#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <map>
#include <string>
#include <vector>

#include <vsg/core/Object.h>

namespace vsg
{

    class Options;
    using Path = std::string;
    using Paths = std::vector<Path>;
    using PathObjects = std::map<Path, ref_ptr<Object>>;

    extern VSG_DECLSPEC std::string getEnv(const char* env_var);
    extern VSG_DECLSPEC Paths getEnvPaths(const char* env_var);

    template<typename... Args>
    Paths getEnvPaths(const char* env_var, Args... args)
    {
        auto paths = getEnvPaths(env_var);
        auto right_paths = getEnvPaths(args...);
        paths.insert(paths.end(), right_paths.begin(), right_paths.end());
        return paths;
    }

    extern VSG_DECLSPEC bool fileExists(const Path& path);

    extern VSG_DECLSPEC Path filePath(const Path& path);

    extern VSG_DECLSPEC Path fileExtension(const Path& path);
    extern VSG_DECLSPEC Path lowerCaseFileExtension(const Path& path);

    extern VSG_DECLSPEC Path simpleFilename(const Path& path);

    extern VSG_DECLSPEC Path removeExtension(const Path& path);

    extern VSG_DECLSPEC Path concatPaths(const Path& left, const Path& right);

    template<typename... Args>
    Path concatPaths(const Path& left, Args... args)
    {
        return concatPaths(left, concatPaths(args...));
    }

    extern VSG_DECLSPEC Path findFile(const Path& filename, const Paths& paths);

    extern VSG_DECLSPEC Path findFile(const Path& filename, const Options* options);

    /// make a directory, return true if path already exists or full path has been created successfully, return false on failure.
    extern VSG_DECLSPEC bool makeDirectory(const Path& path);

} // namespace vsg
