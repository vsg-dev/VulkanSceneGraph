
#include <vsg/io/Options.h>
#include <vsg/utils/SharedObjects.h>

#include <iostream>

using namespace vsg;

SharedObjects::SharedObjects()
{
}

SharedObjects::~SharedObjects()
{
}

void SharedObjects::clear()
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _defaults.clear();
    _sharedObjects.clear();
}

void SharedObjects::report(std::ostream& out)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    out << "SharedObjects::report(..) " << this << std::endl;
    out << "SharedObjects::_defaults " << _defaults.size() << std::endl;
    out << "SharedObjects::_sharedObjects " << SharedObjects::_sharedObjects.size() << std::endl;
}
