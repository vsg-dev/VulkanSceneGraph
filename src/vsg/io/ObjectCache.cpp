/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectCache.h>

using namespace vsg;

void ObjectCache::removeExpiredUnusedObjects()
{
    // TODO get current timestamp to use as a reference for expiry
    auto time = vsg::clock::now();

    std::lock_guard<std::mutex> guard(_mutex);
    for(auto itr = _objectCacheMap.begin(); itr != _objectCacheMap.end();)
    {
        auto current_itr = itr++;
        ObjectTimepoint& ot = current_itr->second;
        if (ot.object->referenceCount()>1)
        {
            ot.lastUsedTimepoint = time;
        }
        else
        {
            // TODO need to check if expired
            auto timeSinceLasUsed = std::chrono::duration<double, std::chrono::seconds::period>(time - ot.lastUsedTimepoint).count();
            if (timeSinceLasUsed > ot.unusedDurationBeforeExpiry)
            {
                _objectCacheMap.erase(current_itr);
            }
        }
    }
}

void ObjectCache::clear()
{
    std::lock_guard<std::mutex> guard(_mutex);

    // remove all objects from cache
    _objectCacheMap.clear();
}

ref_ptr<Object> ObjectCache::get(const Path& filename, ref_ptr<const Options> options)
{
    std::lock_guard<std::mutex> guard(_mutex);

    FilenameOption filenameOption(filename, options);
    if (auto itr = _objectCacheMap.find(filenameOption); itr != _objectCacheMap.end())
    {
        ObjectTimepoint& ot = itr->second;
        ot.lastUsedTimepoint = vsg::clock::now();
        return ot.object;
    }
    else
    {
        return ref_ptr<Object>();
    }
}

void ObjectCache::add(ref_ptr<Object> object, const Path& filename, ref_ptr<const Options> options)
{
    std::lock_guard<std::mutex> guard(_mutex);

    double duration = _defaultUnusedDuration;
    auto time = vsg::clock::now();
    FilenameOption filenameOption(filename, options);
    if (auto itr = _objectCacheMap.find(filenameOption); itr == _objectCacheMap.end())
    {
        _objectCacheMap[filenameOption] = {object, duration, time};
    }
}


