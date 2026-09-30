#include <vsg/vk/Context.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/PhysicalDevice.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

int main()
{
    auto instance = vsg::Instance::create(vsg::Names{}, vsg::Names{});
    auto physicalDevice = instance->getPhysicalDevice(VK_QUEUE_GRAPHICS_BIT);
    if (!physicalDevice)
    {
        std::cerr << "No Vulkan graphics device is available\n";
        return 77;
    }

    int queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    auto device = vsg::Device::create(physicalDevice, vsg::QueueSettings{{queueFamily, {1.0f}}}, vsg::Names{}, vsg::Names{});
    auto context = vsg::Context::create(device);
    auto layout = vsg::DescriptorSetLayout::create();
    layout->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    layout->compile(*context);

    auto pool = vsg::DescriptorPool::create(device, 1, vsg::DescriptorPoolSizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}});
    auto current = pool->allocateDescriptorSet(layout);
    if (!current)
    {
        std::cerr << "Initial descriptor set allocation failed\n";
        return 1;
    }

    for (int iteration = 0; iteration < 5000; ++iteration)
    {
        auto previous = std::move(current);
        std::atomic<bool> recyclingStarted{false};

        std::thread allocationThread([&] {
            while (!recyclingStarted.load(std::memory_order_acquire)) std::this_thread::yield();
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
            while (!(current = pool->allocateDescriptorSet(layout)) && std::chrono::steady_clock::now() < deadline)
                std::this_thread::yield();
        });

        std::thread recyclingThread([&] {
            recyclingStarted.store(true, std::memory_order_release);
            vsg::DescriptorSet::Implementation::recycle(previous);
        });

        recyclingThread.join();
        allocationThread.join();
        if (!current)
        {
            std::cerr << "Descriptor set reuse failed at iteration " << iteration << '\n';
            return 1;
        }
    }

    return 0;
}
