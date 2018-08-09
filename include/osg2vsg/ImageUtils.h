#pragma once

#include <osg/ImageUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <vsg/vk/PhysicalDevice.h>
#include <vsg/vk/Device.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Descriptor.h>

namespace osg2vsg
{
    using GLtoVkFormatMap = std::map<std::pair<GLenum, GLenum>, VkFormat>;
    static GLtoVkFormatMap s_GLtoVkFormatMap = {
        {{GL_UNSIGNED_BYTE, GL_ALPHA}, VK_FORMAT_R8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_LUMINANCE}, VK_FORMAT_R8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_LUMINANCE_ALPHA}, VK_FORMAT_R8G8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_RGB}, VK_FORMAT_R8G8B8_UNORM},
        {{GL_UNSIGNED_BYTE, GL_RGBA}, VK_FORMAT_R8G8B8A8_UNORM}
    };

    VkFormat convertGLImageFormatToVulkan(GLenum dataType, GLenum pixelFormat);

    vsg::ref_ptr<osg::Image> formatImage(osg::Image* image, GLenum pixelFormat);

    vsg::ImageData readImageFile(vsg::PhysicalDevice* physicalDevice, vsg::Device* device, vsg::CommandPool* commandPool, VkQueue graphicsQueue, const std::string& filename);
}

