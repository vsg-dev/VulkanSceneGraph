/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Auxiliary.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Output.h>

using namespace vsg;

Auxiliary::Auxiliary(Object* object) :
    _referenceCount(0),
    _connectedObject(object)
{
    //vsg::debug("Auxiliary::Auxiliary(Object = ", object, ") ", this);
}

Auxiliary::~Auxiliary()
{
    //vsg::debug("Auxiliary::~Auxiliary() ", this);
}

void Auxiliary::ref() const
{
    ++_referenceCount;
    //debug("Auxiliary::ref() ", this, " ", _referenceCount.load());
}

void Auxiliary::unref() const
{
    //debug("Auxiliary::unref() ", this, " ", _referenceCount.load());
    if (_referenceCount.fetch_sub(1) <= 1)
    {
        delete this;
    }
}

void Auxiliary::unref_nodelete() const
{
    //debug("Auxiliary::unref_nodelete() ", this, " ", _referenceCount.load());
    --_referenceCount;
}

bool Auxiliary::signalConnectedObjectToBeDeleted()
{
    std::scoped_lock<std::mutex> guard(_mutex);

    if (_connectedObject && _connectedObject->referenceCount() > 0)
    {
        // return false, the object should not be deleted
        return false;
    }

    // disconnect this Auxiliary object from the ConnectedObject
    _connectedObject = nullptr;

    // return true, the object should be deleted
    return true;
}

void Auxiliary::resetConnectedObject()
{
    std::scoped_lock<std::mutex> guard(_mutex);

    _connectedObject = nullptr;
}

int Auxiliary::compare(const Auxiliary& rhs) const
{
    auto lhs_itr = userObjects.begin();
    auto rhs_itr = rhs.userObjects.begin();
    while (lhs_itr != userObjects.end() && rhs_itr != rhs.userObjects.end())
    {
        if (lhs_itr->first < rhs_itr->first) return -1;
        if (lhs_itr->first > rhs_itr->first) return 1;
        if (int result = vsg::compare_pointer(lhs_itr->second, rhs_itr->second); result != 0) return result;
        ++lhs_itr;
        ++rhs_itr;
    }

    // only can get here if either lhs_itr == userObjects.end() || rhs_itr == rhs.userObjects.end()
    if (lhs_itr == userObjects.end())
    {
        if (rhs_itr != rhs.userObjects.end())
            return -1;
        else
            return 0;
    }
    else
    {
        return 1;
    }
}
