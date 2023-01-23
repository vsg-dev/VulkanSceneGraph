#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <map>
#include <vector>

#include <vsg/core/Object.h>
#include <vsg/core/contains.h>
#include <vsg/io/Path.h>

namespace vsg
{

    class Options;
    using Paths = std::vector<Path>;
    using PathObjects = std::map<Path, ref_ptr<Object>>;

    /// get the specified environmental variable.
    extern VSG_DECLSPEC std::string getEnv(const char* env_var);

    /// parse the specified environmental variable using platorm specific delimiter, returning list of Paths
    /// delimiter used is ; under Windows, and : on all other platforms.
    extern VSG_DECLSPEC Paths getEnvPaths(const char* env_var);

    /// parsing multiple environmental variables, parsing them to return a list of Paths.
    template<typename... Args>
    Paths getEnvPaths(const char* env_var, Args... args)
    {
        auto paths = getEnvPaths(env_var);
        auto right_paths = getEnvPaths(args...);
        paths.insert(paths.end(), right_paths.begin(), right_paths.end());
        return paths;
    }

    /// return file type, see include/vsg/io/Path.h for FileType enum,
    extern VSG_DECLSPEC FileType fileType(const Path& path);

    /// return true if a specified file/path exist on system.
    extern VSG_DECLSPEC bool fileExists(const Path& path);

    /// return path stripped of the filename or final path component.
    extern VSG_DECLSPEC Path filePath(const Path& path);

    /// return file extension include the . prefix, i.e. vsg::fileExtension("file.vsgt") returns .vsgt
    extern VSG_DECLSPEC Path fileExtension(const Path& path);

    /// return lower case file extension include the . prefix, i.e. vsg::fileExtension("file.VSGT") returns .vsgt
    /// By default prunes extras such as REST strings at the end of the extensions, uses ? as the deliminator for REST additions i.e. ".jpeg?g=42" becomes ".jpeg"
    extern VSG_DECLSPEC Path lowerCaseFileExtension(const Path& path, bool pruneExtras = true);

    /// return the filename stripped of any paths and extensions, i.e vsg::simpleFilname("path/file.vsgb") returns file
    extern VSG_DECLSPEC Path simpleFilename(const Path& path);

    /// return the path minus the extension, i.e. vsg::removeExtension("path/file.png") return path/file
    extern VSG_DECLSPEC Path removeExtension(const Path& path);

    /// return true if the path equals ., .. or has a trailing \.. \.., /.. or /....
    extern VSG_DECLSPEC bool trailingRelativePath(const Path& path);

    /// return the full filename path if specified filename can be found in the list of paths.
    extern VSG_DECLSPEC Path findFile(const Path& filename, const Paths& paths);

    /// return the full filename path if specified filename can be found in the options->paths list.
    /// If options is null and the filename can be found using it's existing path that filename is return, otherwise empty Path{} is returned.
    extern VSG_DECLSPEC Path findFile(const Path& filename, const Options* options);

    /// make a directory, return true if path already exists or full path has been created successfully, return false on failure.
    extern VSG_DECLSPEC bool makeDirectory(const Path& path);

    /// get the contents of a directory, return {} if directory name is not a directory
    extern VSG_DECLSPEC Paths getDirectoryContents(const Path& directoryName);

    /// returns the path/filename of the currently executed program.
    extern VSG_DECLSPEC Path executableFilePath();

    /// Open a file using a the C style fopen() adapted with work with the vsg::Path.
    extern VSG_DECLSPEC FILE* fopen(const Path& path, const char* mode);

} // namespace vsg
