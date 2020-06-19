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
#include <vsg/ui/UIEvent.h>

namespace vsg
{

    class ObjectCache : public Inherit<Object, ObjectCache>
    {
    public:
        void setDefaultUnusedDuration(double duration) { _defaultUnusedDuration = duration; }
        double getDefaultUnusedDuration() const { return _defaultUnusedDuration; }

        /// remove any objects that no longer have an external references from cache.that are haven't been referenced within their expiry time
        void removeExpiredUnusedObjects();

        /// remove all objects from cache
        void clear();

        /// check if a cache entry contains an entry for specified filename.
        bool contains(const Path& filename, ref_ptr<const Options> options = {});

        /// get entry from ObjectCache that matches filename and option. return null when no object matches.
        ref_ptr<Object> get(const Path& filename, ref_ptr<const Options> options = {});

        /// add entry from ObjectCache that matches filename and option.
        void add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options = {});

        /// remove entry matching filename and option.
        void remove(const Path& filename, ref_ptr<const Options> options = {});

        /// remove entry matching object.
        void remove(ref_ptr<Object> object);

        struct ObjectTimepoint
        {
            std::mutex mutex;
            ref_ptr<Object> object;
            double unusedDurationBeforeExpiry = 0.0;
            clock::time_point lastUsedTimepoint;
        };

        inline ObjectTimepoint& getObjectTimepoint(const Path& filename, ref_ptr<const Options> options = {})
        {
            std::scoped_lock<std::mutex> guard(_mutex);
            return _objectCacheMap[FilenameOption(filename, options)];
        }

    protected:
        using FilenameOption = std::pair<Path, ref_ptr<const Options>>;
        using ObjectCacheMap = std::map<FilenameOption, ObjectTimepoint>;

        mutable std::mutex _mutex;
        double _defaultUnusedDuration = 0.0;
        ObjectCacheMap _objectCacheMap;
    };
    VSG_type_name(vsg::ObjectCache);

} // namespace vsg
