#include <vsg/vk/Pipeline.h>

namespace vsg
{

////////////////////////////////////////////////////////////////////////
//
// Pipeline
//
Pipeline::Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _bindPoint(bindPoint),
    _device(device),
    _allocator(allocator)
{
}

Pipeline::~Pipeline()
{
    if (_pipeline)
    {
        vkDestroyPipeline(*_device, _pipeline, _allocator);
    }
}


}
