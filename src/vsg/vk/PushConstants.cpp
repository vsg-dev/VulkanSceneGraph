#include <vsg/vk/PushConstants.h>

namespace vsg
{

PushConstants::PushConstants(VkShaderStageFlags shaderFlags, uint32_t offset, Data* data):
    _stageFlags(shaderFlags),
    _offset(offset),
    _data(data)
{
}

PushConstants::~PushConstants()
{
}

}
