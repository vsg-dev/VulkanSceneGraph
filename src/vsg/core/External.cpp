/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/External.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>
#include <vsg/io/read.h>
#include <vsg/io/write.h>

#include <unordered_map>

using namespace vsg;

class CollectIDs : public ConstVisitor
{
public:
    CollectIDs() {}

    void apply(const Object& object) override
    {
        auto itr = _objectIDMap.find(&object);
        if (itr == _objectIDMap.end())
        {
            ObjectID id = _objectID++;
            _objectIDMap[&object] = id;
            object.traverse(*this);
        }
    }

    using ObjectID = uint32_t;
    using ObjectIDMap = std::unordered_map<const Object*, ObjectID>;

    ObjectID _objectID = 0;
    ObjectIDMap _objectIDMap;

    struct ObjectIDRange
    {
        const Object* object = nullptr;
        ObjectID startID = 0;
        ObjectID endID = 0;
    };

    using ObjectIDRangeMap = std::unordered_map<Path, ObjectIDRange>;
    ObjectIDRangeMap objectIDRangeMap;
};

External::External()
{
}

External::External(Allocator* allocator) :
    Inherit(allocator)
{
}

External::External(const PathObjects& in_entries) :
    entries(in_entries)
{
}

External::External(const std::string& filename, ref_ptr<Object> object) :
    entries{{filename, object}}
{
}

External::~External()
{
}

void External::read(Input& input)
{
    entries.clear();

    Object::read(input);

    input.read("options", options);

    CollectIDs collectIDs;

    uint32_t count = input.readValue<uint32_t>("NumEntries");
    Paths filenames(count);
    for (auto& filename : filenames)
    {
        CollectIDs::ObjectIDRange objectIDRange;
        input.read("StartID_EndID_Filename", objectIDRange.startID, objectIDRange.endID, filename);
        collectIDs.objectIDRangeMap[filename] = objectIDRange;
    }

    if (options)
    {
        entries = vsg::read(filenames, options);
    }
    else
    {
        entries = vsg::read(filenames, input.options);
    }

    // collect the ids from the files
    for (auto itr = entries.begin(); itr != entries.end(); ++itr)
    {
        auto& objectIDRange = collectIDs.objectIDRangeMap[itr->first];
        collectIDs._objectID = objectIDRange.startID;
        if (itr->second)
            itr->second->accept(collectIDs);
        else
        {
            for (uint32_t objectID = objectIDRange.startID; objectID <= objectIDRange.endID; ++objectID)
            {
                input.objectIDMap[objectID] = nullptr;
            }
        }
    }

    for (auto [object, objectID] : collectIDs._objectIDMap)
    {
        input.objectIDMap[objectID] = const_cast<Object*>(object);
    }
}

void External::write(Output& output) const
{
    Object::write(output);

    output.write("options", options);

    CollectIDs collectIDs;
    collectIDs._objectID = output.objectID;

    for (auto& [filename, externalObject] : entries)
    {
        if (!filename.empty() && externalObject)
        {
            auto startObjectID = collectIDs._objectID;
            externalObject->accept(collectIDs);
            collectIDs.objectIDRangeMap[filename] = CollectIDs::ObjectIDRange{externalObject, startObjectID, collectIDs._objectID};
        }
        else
        {
            collectIDs.objectIDRangeMap[filename] = CollectIDs::ObjectIDRange{nullptr, collectIDs._objectID, collectIDs._objectID};
        }
    }
    uint32_t idEnd = collectIDs._objectID;
    output.objectID = idEnd;

    // pass the object id's onto the output's objectIDMap
    for (auto& [object, objectID] : collectIDs._objectIDMap)
    {
        output.objectIDMap[object] = objectID;
    }

    output.writeValue<uint32_t>("NumEntries", entries.size());
    for (auto itr = entries.begin(); itr != entries.end(); ++itr)
    {
        auto& objectIDRange = collectIDs.objectIDRangeMap[itr->first];
        output.write("StartID_EndID_Filename", objectIDRange.startID, objectIDRange.endID, itr->first);
    }

    // write out files.
    for (auto& [filename, externalObject] : entries)
    {
        // if we should write out object then need to invoke ReaderWriter for it.
        if (!filename.empty() && externalObject)
        {
            vsg::write(externalObject, filename, output.options);
        }
    }
}
