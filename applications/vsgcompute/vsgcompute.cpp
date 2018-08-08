#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/nodes/Group.h>

#include <vsg/maths/transform.h>

#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ComputePipeline.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSet.h>
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
    bool debugLayer = false;
    bool apiDumpLayer = false;
    uint32_t width = 3200;
    uint32_t height = 2400;
    uint32_t workgroupSize = 32;
    bool useStagingBuffer = false;
    std::string outputFIlename;

    try
    {
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--debug","-d"))) debugLayer = true;
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--api","-a"))) { apiDumpLayer = true; debugLayer = true; }
        if (vsg::CommandLine::read(argc, argv, "--size", width, height)) {}
        if (vsg::CommandLine::read(argc, argv, "-s")) { useStagingBuffer = true; }
        if (vsg::CommandLine::read(argc, argv, "-o", outputFIlename)) {}
        if (vsg::CommandLine::read(argc, argv, "-w", workgroupSize)) {}
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    vsg::Names instanceExtensions;
    vsg::Names requestedLayers;
    vsg::Names deviceExtensions;
    if (debugLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        if (apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

    vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

    vsg::ref_ptr<vsg::Instance> instance = vsg::Instance::create(instanceExtensions, validatedNames);
    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(instance, VK_QUEUE_COMPUTE_BIT);
    vsg::ref_ptr<vsg::Device> device = vsg::Device::create(instance, physicalDevice, validatedNames, deviceExtensions);

    // get the queue for the compute commands
    VkQueue computeQueue = device->getQueue(physicalDevice->getComputeFamily());


    // allocate output storage buffer
    VkDeviceSize bufferSize = sizeof(vsg::vec4) * width * height;
    vsg::ref_ptr<vsg::Buffer> buffer =  vsg::Buffer::create(device, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory>  bufferMemory = vsg::DeviceMemory::create(physicalDevice, device, buffer,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(*device, *buffer, *bufferMemory, 0);


    // set up DescriptSetLayout
    vsg::DescriptorSetLayoutBindings descriptorLayoutBinding(1);
    descriptorLayoutBinding[0].binding = 0;
    descriptorLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorLayoutBinding[0].descriptorCount = 1;
    descriptorLayoutBinding[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout = vsg::DescriptorSetLayout::create(device, descriptorLayoutBinding);


    // set up DescriptorPool and DecriptorSet
    vsg::DescriptorPoolSizes poolSizes{
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool = vsg::DescriptorPool::create(device, 1, poolSizes);
    vsg::ref_ptr<vsg::DescriptorSet> descriptorSet = vsg::DescriptorSet::create(device, descriptorPool, descriptorSetLayout);

    VkDescriptorBufferInfo descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = *buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    std::vector<VkWriteDescriptorSet> descriptorWrites(1);

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = *descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;

    vkUpdateDescriptorSets(*device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

    vsg::ref_ptr<vsg::ShaderModule> computeShader = vsg::ShaderModule::read(device, VK_SHADER_STAGE_COMPUTE_BIT, "main", "shaders/comp.spv");
    if (!computeShader)
    {
        std::cout<<"Could not create shaders"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = vsg::PipelineLayout::create(device, {descriptorSetLayout}, {});

    // set up compute pipeline
    vsg::ref_ptr<vsg::ComputePipeline> pipeline = vsg::ComputePipeline::create(device, pipelineLayout, computeShader);

    // set up bind descriptors
    vsg::ref_ptr<vsg::CmdBindDescriptorSets> bindDescriptorSets = new vsg::CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, {descriptorSet});

    // setup command pool
    vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(device, physicalDevice->getComputeFamily());

    // setup fence
    vsg::ref_ptr<vsg::Fence> fence = vsg::Fence::create(device);

    auto startTime =std::chrono::steady_clock::now();

    // dispatch commands
    vsg::dispatchCommandsToQueue(device, commandPool, fence, 100000000000, computeQueue, [&](VkCommandBuffer commandBuffer)
    {
        pipeline->dispatch(commandBuffer);
        bindDescriptorSets->dispatch(commandBuffer);
        vkCmdDispatch(commandBuffer, uint32_t(ceil(float(width)/float(workgroupSize))), uint32_t(ceil(float(height)/float(workgroupSize))), 1);
    });

    auto time = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::steady_clock::now()-startTime).count();
    std::cout<<"Time to run commands "<<time<<"ms"<<std::endl;

    if (!outputFIlename.empty())
    {
        void* mappedMemory = NULL;
        vkMapMemory(*device, *bufferMemory, 0, bufferSize, 0, &mappedMemory);

        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        float* src_ptr = reinterpret_cast<float*>(mappedMemory);
        unsigned char* dest_ptr = image->data();
        for(int i=0; i<width*height; ++i)
        {
            (*dest_ptr++) = (unsigned char)((*src_ptr++)*255.0f);
            (*dest_ptr++) = (unsigned char)((*src_ptr++)*255.0f);
            (*dest_ptr++) = (unsigned char)((*src_ptr++)*255.0f);
            (*dest_ptr++) = (unsigned char)((*src_ptr++)*255.0f);
        }

        osgDB::writeImageFile(*image, outputFIlename);

        vkUnmapMemory(*device, *bufferMemory);
    }


    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
