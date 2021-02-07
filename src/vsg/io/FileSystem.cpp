/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#    include <cstdlib>
#    include <direct.h>
#    include <io.h>
#else
#    include <sys/stat.h>
#    include <unistd.h>
#endif

#include <iostream>

using namespace vsg;

const char UNIX_PATH_SEPARATOR = '/';
const char WINDOWS_PATH_SEPARATOR = '\\';
const char* const PATH_SEPARATORS = "/\\";

#if defined(WIN32) && !defined(__CYGWIN__)
const char delimiterNative = WINDOWS_PATH_SEPARATOR;
const char delimiterForeign = UNIX_PATH_SEPARATOR;
const char envPathDelimiter = ';';
#else
const char delimiterNative = UNIX_PATH_SEPARATOR;
const char delimiterForeign = WINDOWS_PATH_SEPARATOR;
const char envPathDelimiter = ':';
#endif

Paths vsg::getEnvPaths(const char* env_var)
{
    Paths filepaths;
    if (!env_var) return filepaths;

#if defined(WIN32) && !defined(__CYGWIN__)
    char env_value[4096];
    std::size_t len;
    if (auto error = getenv_s(&len, env_value, sizeof(env_value) - 1, env_var); error != 0 || len == 0)
    {
        return filepaths;
    }
#else
    const char* env_value = getenv(env_var);
    if (env_value == nullptr) return filepaths;
#endif

    std::string paths(env_value);

    std::string::size_type start = 0;
    std::string::size_type end;
    while ((end = paths.find_first_of(envPathDelimiter, start)) != std::string::npos)
    {
        filepaths.push_back(paths.substr(start, end - start));
        start = end + 1;
    }

    std::string lastPath(paths, start, std::string::npos);
    if (!lastPath.empty())
        filepaths.push_back(lastPath);

    return filepaths;
}

bool vsg::fileExists(const Path& path)
{
#if defined(WIN32)
    return _access(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

Path vsg::filePath(const Path& path)
{
    std::string::size_type slash = path.find_last_of(PATH_SEPARATORS);
    if (slash != std::string::npos)
    {
        return path.substr(0, slash);
    }
    else
    {
        return Path();
    }
}

Path vsg::fileExtension(const Path& path)
{
    std::string::size_type dot = path.find_last_of('.');
    std::string::size_type slash = path.find_last_of(PATH_SEPARATORS);
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return Path();
    return path.substr(dot + 1);
}

Path vsg::lowerCaseFileExtension(const Path& path)
{
    Path ext = fileExtension(path);
    for (auto& c : ext) c = std::tolower(c);
    return ext;
}

Path vsg::simpleFilename(const Path& path)
{
    std::string::size_type dot = path.find_last_of('.');
    std::string::size_type slash = path.find_last_of(PATH_SEPARATORS);
    if (slash != std::string::npos)
    {
        if ((dot == std::string::npos) || (dot < slash))
            return path.substr(slash + 1);
        else
            return path.substr(slash + 1, dot - slash - 1);
    }
    else
    {
        if (dot == std::string::npos)
            return path;
        else
            return path.substr(0, dot);
    }
}

Path vsg::removeExtension(const Path& path)
{
    std::string::size_type dot = path.find_last_of('.');
    std::string::size_type slash = path.find_last_of(PATH_SEPARATORS);
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return path;
    else if (dot > 1)
        return path.substr(0, dot);
    else
        return {};
}

Path vsg::concatPaths(const Path& left, const Path& right)
{
    if (left.empty())
    {
        return (right);
    }
    char lastChar = left[left.size() - 1];

    if (lastChar == delimiterNative)
    {
        return left + right;
    }
    else if (lastChar == delimiterForeign)
    {
        return left.substr(0, left.size() - 1) + delimiterNative + right;
    }
    else // lastChar != a delimiter
    {
        return left + delimiterNative + right;
    }
}

Path vsg::findFile(const Path& filename, const Paths& paths)
{
    for (auto path : paths)
    {
        Path fullpath = concatPaths(path, filename);
        if (fileExists(fullpath))
        {
            return fullpath;
        }
    }
    return {};
}

Path vsg::findFile(const Path& filename, const Options* options)
{
    if (options && !options->paths.empty())
    {
        if (options->checkFilenameHint == Options::CHECK_ORIGINAL_FILENAME_EXISTS_FIRST && fileExists(filename)) return filename;

        if (auto path = findFile(filename, options->paths); !path.empty()) return path;

        if (options->checkFilenameHint == Options::CHECK_ORIGINAL_FILENAME_EXISTS_LAST && fileExists(filename))
            return filename;
        else
            return {};
    }
    else
    {
        return fileExists(filename) ? filename : Path();
    }
}

bool vsg::makeDirectory(const Path& path)
{
    std::vector<vsg::Path> directoriesToCreate;
    Path trimmed_path = path;
    while (!trimmed_path.empty() && !vsg::fileExists(trimmed_path))
    {
        directoriesToCreate.push_back(trimmed_path);
        trimmed_path = vsg::filePath(trimmed_path);
    }

    for (auto itr = directoriesToCreate.rbegin(); itr != directoriesToCreate.rend(); ++itr)
    {
        vsg::Path directory_to_create = *itr;

        if (directory_to_create.size() == 2 && directory_to_create[1] == ':')
        {
            // ignore a C: style drive prefixes
            continue;
        }

#if defined(WIN32) && !defined(__CYGWIN__)
        if (int status = _mkdir(directory_to_create.c_str()); status != 0)
        {
            std::cerr << "   _mkdir(" << directory_to_create << ") failed. status = " << status << std::endl;
            return false;
        }
#else
        if (int status = mkdir(directory_to_create.c_str(), 0755); status != 0)
        {
            std::cerr << "   mkdir(" << directory_to_create << ") failed. status = " << status << std::endl;
            return false;
        }
#endif
    }

    return true;
}
