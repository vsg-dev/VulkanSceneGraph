
#include <vsg/utils/SharedObjects.h>
#include <vsg/io/Options.h>

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
    _pool.clear();
    _sharedObjects.clear();
}

void SharedObjects::report(std::ostream& out)
{
    std::cout<<"SharedObjects::report(..) "<<this<<std::endl;
    std::cout<<"SharedObjects::_defaults "<<_defaults.size()<<std::endl;
    std::cout<<"SharedObjects::_pool "<<_pool.size()<<std::endl;
    std::cout<<"SharedObjects::_sharedObjects "<<SharedObjects::_sharedObjects.size()<<std::endl;
}
