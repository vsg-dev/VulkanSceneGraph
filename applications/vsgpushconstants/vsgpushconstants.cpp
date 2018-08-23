#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Result.h>

#include <vsg/utils/CommandLine.h>

#include <vsg/nodes/Group.h>

#include <vsg/maths/transform.h>

#include <vsg/vk/Draw.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/Sampler.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/PushConstants.h>

#include <vsg/viewer/Window.h>
#include <vsg/viewer/Viewer.h>

#include <vsg/utils/stream.h>
#include <vsg/utils/FileSystem.h>

#include <osg2vsg/ImageUtils.h>

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
    uint32_t width = 800;
    uint32_t height = 600;
    int numFrames=-1;
    bool printFrameRate = false;
    int numWindows = 1;

    try
    {
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--debug","-d"))) debugLayer = true;
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--api","-a"))) { apiDumpLayer = true; debugLayer = true; }
        if (vsg::CommandLine::read(argc, argv, vsg::CommandLine::Match("--window","-w"), width, height)) {}
        if (vsg::CommandLine::read(argc, argv, "-f", numFrames)) {}
        if (vsg::CommandLine::read(argc, argv, "--fr")) { printFrameRate = true; }
        if (vsg::CommandLine::read(argc, argv, "--num-windows", numWindows)) {}
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");

    vsg::ref_ptr<vsg::Shader> vertexShader = vsg::Shader::read( VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert_PushConstants.spv", searchPaths));
    vsg::ref_ptr<vsg::Shader> fragmentShader = vsg::Shader::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag_PushConstants.spv", searchPaths));
    if (!vertexShader || !fragmentShader)
    {
        std::cout<<"Could not create shaders."<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::Viewer> viewer = new vsg::Viewer;

    vsg::ref_ptr<vsg::Window> window = vsg::Window::create(width, height, debugLayer, apiDumpLayer);
    if (!window)
    {
        std::cout<<"Could not create windows."<<std::endl;
        return 1;
    }

    viewer->addWindow(window);

    for(int i=1; i<numWindows; ++i)
    {
        vsg::ref_ptr<vsg::Window> new_window = vsg::Window::create(width, height, debugLayer, apiDumpLayer, window);
        viewer->addWindow( new_window );
    }

    vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = window->physicalDevice();
    vsg::ref_ptr<vsg::Device> device = window->device();
    vsg::ref_ptr<vsg::Surface> surface = window->surface();
    vsg::ref_ptr<vsg::RenderPass> renderPass = window->renderPass();

    std::cout<<"maxPushConstantsSize="<<physicalDevice->getProperties().limits.maxPushConstantsSize<<std::endl;


    VkQueue graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
    VkQueue presentQueue = device->getQueue(physicalDevice->getPresentFamily());
    if (!graphicsQueue || !presentQueue)
    {
        std::cout<<"Required graphics/present queue not available!"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::CommandPool> commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());


    // set up vertex and index arrays
    vsg::ref_ptr<vsg::vec3Array> vertices = new vsg::vec3Array
    {
        {-0.5f, -0.5f, 0.0f},
        {0.5f,  -0.5f, 0.05f},
        {0.5f , 0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f},
        {-0.5f, -0.5f, -0.5f},
        {0.5f,  -0.5f, -0.5f},
        {0.5f , 0.5f, -0.5},
        {-0.5f, 0.5f, -0.5}
    };

    vsg::ref_ptr<vsg::vec3Array> colors = new vsg::vec3Array
    {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
    };

    vsg::ref_ptr<vsg::vec2Array> texcoords = new vsg::vec2Array
    {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    vsg::ref_ptr<vsg::ushortArray> indices = new vsg::ushortArray
    {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4
    };

    auto vertexBufferData = vsg::createBufferAndTransferData(device, commandPool, graphicsQueue, {vertices, colors, texcoords}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferData = vsg::createBufferAndTransferData(device, commandPool, graphicsQueue, {indices}, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);


    //
    // set up texture image
    //
    vsg::ImageData imageData = osg2vsg::readImageFile(device, commandPool, graphicsQueue, vsg::findFile("textures/lz.rgb", searchPaths));
    if (!imageData.valid())
    {
        std::cout<<"Texture not created"<<std::endl;
        return 1;
    }

    vsg::ref_ptr<vsg::mat4Value> projMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> viewMatrix = new vsg::mat4Value;
    vsg::ref_ptr<vsg::mat4Value> modelMatrix = new vsg::mat4Value;

    //
    // set up descriptor layout and descriptor set and pieline layout for uniforms
    //
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool = vsg::DescriptorPool::create(device, 1,
    {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
    });

    vsg::ref_ptr<vsg::PushConstants> pushConstant_proj = new vsg::PushConstants(VK_SHADER_STAGE_VERTEX_BIT, 0, projMatrix);
    vsg::ref_ptr<vsg::PushConstants> pushConstant_view = new vsg::PushConstants(VK_SHADER_STAGE_VERTEX_BIT, 64, viewMatrix);
    vsg::ref_ptr<vsg::PushConstants> pushConstant_model = new vsg::PushConstants(VK_SHADER_STAGE_VERTEX_BIT, 128, modelMatrix);

    vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout = vsg::DescriptorSetLayout::create(device,
    {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    });

    vsg::PushConstantRanges pushConstantRanges
    {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 196}
    };

    vsg::ref_ptr<vsg::DescriptorSet> descriptorSet = vsg::DescriptorSet::create(device, descriptorPool, descriptorSetLayout,
    {
        new vsg::DescriptorImage(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, {imageData})
    });

    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout = vsg::PipelineLayout::create(device, {descriptorSetLayout}, pushConstantRanges);


    // setup binding of descriptors
    vsg::ref_ptr<vsg::BindDescriptorSets> bindDescriptorSets = new vsg::BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, {descriptorSet}); // device dependent


    // set up graphics pipeline
    vsg::VertexInputState::Bindings vertexBindingsDescriptions
    {
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
        VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions
    {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    vsg::ref_ptr<vsg::ShaderStages> shaderStages = new vsg::ShaderStages(
    {
        vsg::ShaderModule::create(device, vertexShader),
        vsg::ShaderModule::create(device, fragmentShader)
    });

    vsg::ref_ptr<vsg::GraphicsPipeline> pipeline = vsg::GraphicsPipeline::create(device, renderPass, pipelineLayout, // device dependent
    {
        shaderStages,  // device dependent
        new vsg::VertexInputState(vertexBindingsDescriptions, vertexAttributeDescriptions),// device independent
        new vsg::InputAssemblyState, // device independent
        new vsg::ViewportState(VkExtent2D{width, height}), // device independent
        new vsg::RasterizationState,// device independent
        new vsg::MultisampleState,// device independent
        new vsg::ColorBlendState,// device independent
        new vsg::DepthStencilState// device independent
    });

    vsg::ref_ptr<vsg::BindPipeline> bindPipeline = new vsg::BindPipeline(pipeline);

    // set up vertex buffer binding
    vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers = new vsg::BindVertexBuffers(0, vertexBufferData);  // device dependent

    // set up index buffer binding
    vsg::ref_ptr<vsg::BindIndexBuffer> bindIndexBuffer = new vsg::BindIndexBuffer(indexBufferData.front(), VK_INDEX_TYPE_UINT16); // device dependent

    // set up drawing of the triangles
    vsg::ref_ptr<vsg::DrawIndexed> drawIndexed = new vsg::DrawIndexed(12, 1, 0, 0, 0); // device agnostic

    // set up what we want to render in a command graph
    // create command graph to contain all the Vulkan calls for specifically rendering the model
    vsg::ref_ptr<vsg::Group> commandGraph = new vsg::Group;

    // set up the state configuration
    commandGraph->addChild(bindPipeline);  // device dependent
    commandGraph->addChild(bindDescriptorSets);  // device dependent

    commandGraph->addChild(pushConstant_proj);
    commandGraph->addChild(pushConstant_view);
    commandGraph->addChild(pushConstant_model);

    // add subgraph that represents the model to render
    vsg::ref_ptr<vsg::Group> model = new vsg::Group;
    commandGraph->addChild(model);

    // add the vertex and index buffer data
    model->addChild(bindVertexBuffers); // device dependent
    model->addChild(bindIndexBuffer); // device dependent

    // add the draw primitive command
    model->addChild(drawIndexed); // device independent

    //
    // end of initialize vulkan
    //
    /////////////////////////////////////////////////////////////////////

    auto startTime =std::chrono::steady_clock::now();
    float time = 0.0f;

    for (auto& win : viewer->windows())
    {
        win->populateCommandBuffers(commandGraph);
    }

    while (!viewer->done() && (numFrames<0 || (numFrames--)>0))
    {
        viewer->pollEvents();

        float previousTime = time;
        time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now()-startTime).count();
        if (printFrameRate) std::cout<<"time = "<<time<<" fps="<<1.0/(time-previousTime)<<std::endl;

        (*projMatrix) = vsg::perspective(vsg::radians(45.0f), float(width)/float(height), 0.1f, 10.f);
        (*viewMatrix) = vsg::lookAt(vsg::vec3(2.0f, 2.0f, 2.0f), vsg::vec3(0.0f, 0.0f, 0.0f), vsg::vec3(0.0f, 0.0f, 1.0f));
        (*modelMatrix) = vsg::rotate(time * vsg::radians(90.0f), vsg::vec3(0.0f, 0.0, 1.0f));

        for (auto& win : viewer->windows())
        {
            // we need to regenerate the CommandBuffer so that the PushConstants get called with the new values.
            win->populateCommandBuffers(commandGraph);
        }

        viewer->submitFrame(commandGraph);
    }


    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
