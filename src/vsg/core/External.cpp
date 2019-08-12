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

#include <unordered_map>

#include <iostream>

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
};

External::External()
{
}

External::External(Allocator* allocator) :
    Inherit(allocator)
{
}

External::External(const std::string& filename, ref_ptr<Object> object) :
    _filename(filename),
    _object(object)
{
}

External::~External()
{
}

void External::read(Input& input)
{
    Object::read(input);

    uint32_t idBegin = 0, idEnd = 0;
    input.read("ObjectIDRange", idBegin, idEnd);
    input.read("Filename", _filename);

    if (!_filename.empty())
    {
        _object = input.readFile(_filename);

        if (_object)
        {
            CollectIDs collectIDs;
            collectIDs._objectID = idBegin;
            _object->accept(collectIDs);

            for (auto [object, objectID] : collectIDs._objectIDMap)
            {
                if ((idBegin <= objectID) && (objectID < idEnd))
                {
                    input.getObjectIDMap()[objectID] = const_cast<Object*>(object);
                }
                else
                {
                    std::cout << "External::read() : warning object out of ObjectIDRange " << objectID << ", " << object << std::endl;
                }
            }
        }
    }
}

void External::write(Output& output) const
{
    Object::write(output);

    uint32_t idBegin = output.getObjectID();
    uint32_t idEnd = idBegin;

    // if we should write out object then need to invoke ReaderWriter for it.
    if (!_filename.empty() && _object)
    {
        output.write(_object, _filename);

        CollectIDs collectIDs;
        collectIDs._objectID = idBegin;
        _object->accept(collectIDs);

        for (auto [object, objectID] : collectIDs._objectIDMap)
        {
            output.getObjectIDMap()[object] = objectID;
        }

        output.setObjectID(collectIDs._objectID);
    }
    idEnd = output.getObjectID();

    output.write("ObjectIDRange", idBegin, idEnd);
    output.write("Filename", _filename);
}
