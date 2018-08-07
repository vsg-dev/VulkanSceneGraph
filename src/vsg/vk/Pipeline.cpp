#include <vsg/vk/Pipeline.h>

#include <iostream>

namespace vsg
{

////////////////////////////////////////////////////////////////////////
//
// Pipeline
//
Pipeline::Pipeline(Device* device, VkPipeline pipeline, VkPipelineBindPoint bindPoint, AllocationCallbacks* allocator) :
    _device(device),
    _pipeline(pipeline),
    _bindPoint(bindPoint),
    _allocator(allocator)
{
}

Pipeline::~Pipeline()
{
    if (_pipeline)
    {
        std::cout<<"Calling vkDestroyPipeline"<<std::endl;
        vkDestroyPipeline(*_device, _pipeline, *_allocator);
    }
}


}