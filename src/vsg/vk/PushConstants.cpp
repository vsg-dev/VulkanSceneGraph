#include <vsg/vk/PushConstants.h>

namespace vsg
{

PushConstants::PushConstants(VkShaderStageFlags stageFlags, uint32_t offset, Data* data):
    _stageFlags(stageFlags),
    _offset(offset),
    _data(data)
{
}

PushConstants::~PushConstants()
{
}

}
