#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <string>
#include <vector>

#include <vsg/core/Export.h>

namespace vsg
{

    using Path = std::string;

    using Paths = std::vector<Path>;

    extern VSG_DECLSPEC Paths getEnvPaths(const char* env_var);

    extern VSG_DECLSPEC bool fileExists(const Path& path);

    extern VSG_DECLSPEC Path fileExtension(const Path& path);

    extern VSG_DECLSPEC Path concatPaths(const Path& left, const Path& right);

    extern VSG_DECLSPEC Path findFile(const Path& filename, const Paths& paths);

} // namespace vsg
