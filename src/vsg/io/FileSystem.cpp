/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/FileSystem.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/stream.h>

#include <cstdio>

#if defined(WIN32) && !defined(__CYGWIN__)
#    include <cstdlib>
#    include <direct.h>
#    include <io.h>
//	 cctype is needed for tolower()
#    include <cctype>
#    include <windows.h>

#    ifdef _MSC_VER
#        ifndef PATH_MAX
#            define PATH_MAX MAX_PATH
#        endif
#    endif

#else
#    include <errno.h>
#    include <sys/stat.h>
#    include <unistd.h>
#endif

#ifdef __APPLE__
#    include <TargetConditionals.h>
#    include <libgen.h>
#    include <mach-o/dyld.h>
#endif

#include <limits.h>

using namespace vsg;

#if defined(_MSC_VER)
const char envPathDelimiter = ';';
#else
const char envPathDelimiter = ':';
#endif

std::string vsg::getEnv(const char* env_var)
{
#if defined(_MSC_VER)
    char env_value[4096];
    std::size_t len;
    if (auto error = getenv_s(&len, env_value, sizeof(env_value) - 1, env_var); error != 0 || len == 0)
    {
        return {};
    }
#else
    const char* env_value = getenv(env_var);
    if (env_value == nullptr) return {};
#endif
    return std::string(env_value);
}

Paths vsg::getEnvPaths(const char* env_var)
{
    if (!env_var) return {};

#if defined(_MSC_VER)
    char env_value[4096];
    std::size_t len;
    if (auto error = getenv_s(&len, env_value, sizeof(env_value) - 1, env_var); error != 0 || len == 0)
    {
        return {};
    }
#else
    const char* env_value = getenv(env_var);
    if (env_value == nullptr) return {};
#endif

    Paths filepaths;
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
#if defined(_MSC_VER)
    return _waccess(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), F_OK) == 0;
#endif
}

Path vsg::filePath(const Path& path)
{
    if (trailingRelativePath(path)) return path;

    auto slash = path.find_last_of(Path::separators);
    if (slash != vsg::Path::npos)
    {
        return path.substr(0, slash);
    }
    else
    {
        return {};
    }
}

Path vsg::fileExtension(const Path& path)
{
    auto dot = path.find_last_of('.');
    if (dot == Path::npos || (dot + 1) == path.size()) return {};

    auto slash = path.find_last_of(Path::separators);
    if (slash != Path::npos && dot < slash) return {};

    return path.substr(dot);
}

Path vsg::lowerCaseFileExtension(const Path& path)
{
    Path ext = fileExtension(path);
    for (auto& c : ext) c = std::tolower(c);
    return ext;
}

Path vsg::simpleFilename(const Path& path)
{
    if (trailingRelativePath(path)) return {};

    auto dot = path.find_last_of('.');
    auto slash = path.find_last_of(Path::separators);
    if (slash != Path::npos)
    {
        if ((dot == Path::npos) || (dot < slash))
            return path.substr(slash + 1);
        else
            return path.substr(slash + 1, dot - slash - 1);
    }
    else
    {
        if (dot == Path::npos)
            return path;
        else
            return path.substr(0, dot);
    }
}

bool vsg::trailingRelativePath(const Path& path)
{
    if (path == ".") return true;
    if (path == "..") return true;
    if (path.size() >= 2)
    {
        if (path.compare(path.size() - 2, 2, "/.") == 0) return true;
        if (path.compare(path.size() - 2, 2, "\\.") == 0) return true;

        if (path.size() >= 3)
        {
            if (path.compare(path.size() - 3, 3, "/..") == 0) return true;
            if (path.compare(path.size() - 3, 3, "\\..") == 0) return true;
        }
    }
    return false;
}

Path vsg::removeExtension(const Path& path)
{
    if (trailingRelativePath(path)) return path;

    auto dot = path.find_last_of('.');
    if (dot == Path::npos) return path;

    auto slash = path.find_last_of(Path::separators);
    if (slash != Path::npos && dot < slash)
        return path;
    else if (dot > 1)
        return path.substr(0, dot);
    else
        return {};
}

Path vsg::findFile(const Path& filename, const Paths& paths)
{
    for (auto path : paths)
    {
        Path fullpath = path / filename;
        if (fileExists(fullpath))
        {
            return fullpath;
        }
    }
    return {};
}

Path vsg::findFile(const Path& filename, const Options* options)
{
    if (options)
    {
        // if Options has a findFileCallback use it
        if (options->findFileCallback) return options->findFileCallback(filename, options);

        if (!options->paths.empty())
        {
            // if appropriate use the filename directly if it exists.
            if (options->checkFilenameHint == Options::CHECK_ORIGINAL_FILENAME_EXISTS_FIRST && fileExists(filename)) return filename;

            // search for the file if the in the specific paths.
            if (auto path = findFile(filename, options->paths)) return path;

            // if appropriate use the filename directly if it exists.
            if (options->checkFilenameHint == Options::CHECK_ORIGINAL_FILENAME_EXISTS_LAST && fileExists(filename))
                return filename;
            else
                return {};
        }
    }

    return fileExists(filename) ? filename : Path();
}

bool vsg::makeDirectory(const Path& path)
{
    std::vector<vsg::Path> directoriesToCreate;
    Path trimmed_path = path;
    while (trimmed_path && !vsg::fileExists(trimmed_path))
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

#if defined(_MSC_VER)
        if (int status = _wmkdir(directory_to_create.c_str()); status != 0)
#elif defined(__MINGW32__)
        if (int status = mkdir(directory_to_create.c_str()); status != 0)
#else // POSIX
        if (int status = mkdir(directory_to_create.c_str(), 0755); status != 0)
#endif
        {
            if (errno != EEXIST)
            {
                // quietly ignore a mkdir on a file that already exists as this can happen safely during a filling in a filecache.
                debug("mkdir(", directory_to_create, ") failed. errno = ", errno);
            }
            return false;
        }
    }

    return true;
}

Path vsg::executableFilePath()
{
    Path path;

#if defined(WIN32)
    TCHAR buf[PATH_MAX + 1];
    DWORD result = GetModuleFileName(NULL, buf, static_cast<DWORD>(std::size(buf) - 1));
    if (result && result < std::size(buf))
        path = buf;
#elif defined(__linux__)

    std::vector<char> buffer(1024);
    ssize_t len = 0;
    while ((len = ::readlink("/proc/self/exe", buffer.data(), buffer.size())) == static_cast<ssize_t>(buffer.size()))
    {
        buffer.resize(buffer.size() * 2);
    }

    // add terminator to string.
    buffer[len] = '\0';

    return buffer.data();

#elif defined(__APPLE__)
#    if TARGET_OS_MAC
    char realPathName[PATH_MAX + 1];
    char buf[PATH_MAX + 1];
    uint32_t size = (uint32_t)sizeof(buf);

    if (!_NSGetExecutablePath(buf, &size))
    {
        realpath(buf, realPathName);
        path = realPathName;
    }
#    elif TARGET_IPHONE_SIMULATOR
    // iOS, tvOS, or watchOS Simulator
    // Not currently implemented
#    elif TARGET_OS_MACCATALYST
    // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    // Not currently implemented
#    elif TARGET_OS_IPHONE
    // iOS, tvOS, or watchOS device
    // Not currently implemented
#    else
#        error "Unknown Apple platform"
#    endif
#elif defined(__ANDROID__)
    // Not currently implemented
#endif
    return path;
}

FILE* vsg::fopen(const Path& path, const char* mode)
{
#if defined(_MSC_VER)
    std::wstring wMode;
    convert_utf(mode, wMode);

    FILE* file = nullptr;
    auto errorNo = _wfopen_s(&file, path.c_str(), wMode.c_str());
    if (errorNo == 0)
        return file;
    else
        return nullptr;
#else
    return ::fopen(path.c_str(), mode);
#endif
}
