#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>

namespace vsg
{

    class ObjectCache : public Inherit<Object, ObjectCache>
    {
    public:

        using FilenameOption = std::pair<Path, ref_ptr<const Options>>;
        using ObjectCacheMap = std::map<FilenameOption, ref_ptr<Object>>;

        /// get entry from ObjectCache that matches filename and option. return null when no object matches.
        ref_ptr<Object> get(const Path& filename, ref_ptr<const Options> options = {}) const;

        /// add entry from ObjectCache that matches filename and option.
        void add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options = {});

    protected:

        mutable std::mutex _mutex;
        ObjectCacheMap _objectCacheMap;
    };
    VSG_type_name(vsg::ObjectCache);

} // namespace vsg
