#include <vsg/utils/FileSystem.h>

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace vsg
{

const char UNIX_PATH_SEPARATOR = '/';
const char WINDOWS_PATH_SEPARATOR = '\\';

#if defined(WIN32) && !defined(__CYGWIN__)
    const char delimiterNative  = WINDOWS_PATH_SEPARATOR;
    const char delimiterForeign = UNIX_PATH_SEPARATOR;
    const char envPathDelimiter = ';';
#else
    const char delimiterNative  = UNIX_PATH_SEPARATOR;
    const char delimiterForeign = WINDOWS_PATH_SEPARATOR;
    const char envPathDelimiter = ':';
#endif


Paths getEnvPaths(const char* env_var)
{
    Paths filepaths;
    if (!env_var) return filepaths;

    const char* env_value = getenv(env_var);
    if (env_value!=nullptr)
    {
        std::string paths(env_value);

        std::string::size_type start = 0;
        std::string::size_type end;
        while ((end = paths.find_first_of(envPathDelimiter, start))!=std::string::npos)
        {
            filepaths.push_back(paths.substr(start, end-start));
            start = end+1;
        }

        std::string lastPath(paths,start,std::string::npos);
        if (!lastPath.empty())
            filepaths.push_back(lastPath);
    }

    return filepaths;
}

bool fileExists(const Path& path)
{
#if defined(WIN32)
    return _access( path.c_str(), F_OK ) == 0;
#else
    return access( path.c_str(), F_OK ) == 0;
#endif
}

Path concatePaths(const Path& left, const Path& right)
{
    if(left.empty())
    {
        return(right);
    }
    char lastChar = left[left.size() - 1];

    if(lastChar == delimiterNative)
    {
        return left + right;
    }
    else if(lastChar == delimiterForeign)
    {
        return left.substr(0, left.size() - 1) + delimiterNative + right;
    }
    else // lastChar != a delimiter
    {
        return left + delimiterNative + right;
    }
}

Path findFile(const Path& filename, const Paths& paths)
{
    for(auto path : paths)
    {
        Path fullpath = concatePaths(path, filename);
        if (fileExists(fullpath))
        {
            return fullpath;
        }
    }
    return Path();
}

}
