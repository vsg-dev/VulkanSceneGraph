#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/nodes/Group.h>

#include <vsg/maths/transform.h>

#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Pipeline.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/Sampler.h>

#include <osg/ImageUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <set>
#include <chrono>
#include <cstring>


int main(int argc, char** argv)
{

    vsg::ref_ptr<vsg::Instance> instance;
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = window->physicalDevice();
    vsg::ref_ptr<vsg::Device> device = window->device();

    vsg::ref_ptr<vsg::ShaderModule> computeShader = vsg::ShaderModule::read(device, VK_SHADER_STAGE_VERTEX_BIT, "main", "shaders/compute.spv");
    if (!computeShader)
    {
        std::cout<<"Could not create shaders"<<std::endl;
        return 1;
    }
    vsg::ShaderModules shaderModules{vert, frag};
    vsg::ref_ptr<vsg::ShaderStages> shaderStages = new vsg::ShaderStages({computeShader});

    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
