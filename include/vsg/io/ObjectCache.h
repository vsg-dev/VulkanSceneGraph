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

    class VSG_DECLSPEC ObjectCache : public Inherit<Object, ObjectCache>
    {
    public:
        void setDefaultUnusedDuration(double duration) { _defaultUnusedDuration = duration; }
        double getDefaultUnusedDuration() const { return _defaultUnusedDuration; }

        /// remove any objects that no longer have an external references from cache.that are haven't been referenced within their expiry time
        virtual void removeExpiredUnusedObjects();

        /// remove all objects from cache
        virtual void clear();

        /// check if a cache entry contains an entry for specified filename.
        virtual bool contains(const Path& filename, ref_ptr<const Options> options = {});

        /// get entry from ObjectCache that matches filename and option. return null when no object matches.
        virtual ref_ptr<Object> get(const Path& filename, ref_ptr<const Options> options = {});

        /// add entry from ObjectCache that matches filename and option.
        virtual void add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options = {});

        /// remove entry matching filename and option.
        virtual void remove(const Path& filename, ref_ptr<const Options> options = {});

        /// remove entry matching object.
        virtual void remove(ref_ptr<Object> object);

        /// set of lower case file extensions for file types that should not be included in this ObjectCache
        std::set<Path> excludedExtensions;

        /// return true if the specified filename is of a type suitable for inclusion in the ObjectCache
        virtual bool suitable(const Path& filename) const;

        /// enable/disable use of matching both filename and options object.
        bool requireMatchedOptions = false;

        struct ObjectTimepoint
        {
            std::mutex mutex;
            ref_ptr<Object> object;
            double unusedDurationBeforeExpiry = 0.0;
            clock::time_point lastUsedTimepoint;
        };

        virtual ObjectTimepoint& getObjectTimepoint(const Path& filename, ref_ptr<const Options> options = {});

    protected:
        struct Key
        {
            Key(const Path& in_filename, ref_ptr<const Options> in_options) :
                auxilary(in_options ? in_options->getAuxiliary() : nullptr),
                filename(in_filename) {}

            // use an Options's Auxiliary object to avoid Options->ObjectCache->Options circular references
            ref_ptr<const Auxiliary> auxilary;
            Path filename;

            bool operator<(const Key& rhs) const
            {
                if (auxilary < rhs.auxilary) return true;
                if (rhs.auxilary < auxilary) return false;
                return filename < rhs.filename;
            }
        };

        using ObjectCacheMap = std::map<Key, ObjectTimepoint>;

        mutable std::mutex _mutex;
        double _defaultUnusedDuration = 0.0;
        ObjectCacheMap _objectCacheMap;
    };
    VSG_type_name(vsg::ObjectCache);

} // namespace vsg
