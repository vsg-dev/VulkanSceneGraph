#include <osg2vsg/ImageUtils.h>
#include <vsg/vk/CommandVisitor.h>

namespace osg2vsg
{

VkFormat convertGLImageFormatToVulkan(GLenum dataType, GLenum pixelFormat)
{
    auto itr = s_GLtoVkFormatMap.find({dataType,pixelFormat});
    if (itr!=s_GLtoVkFormatMap.end())
    {
        std::cout<<"convertGLImageFormatToVulkan("<<dataType<<", "<<pixelFormat<<") vkFormat="<<itr->second<<std::endl;
        return itr->second;
    }
    else
    {
        std::cout<<"convertGLImageFormatToVulkan("<<dataType<<", "<<pixelFormat<<") no match found."<<std::endl;
        return VK_FORMAT_UNDEFINED;
    }
}

struct WriteRow : public osg::CastAndScaleToFloatOperation
{
    WriteRow(unsigned char* ptr) : _ptr(ptr) {}
    unsigned char* _ptr;

    inline void luminance(float l) { rgba(l, l, l, 1.0f); }
    inline void alpha(float a) { rgba(1.0f, 1.0f, 1.0f, a); }
    inline void luminance_alpha(float l,float a) { rgba(l, l, l, a); }
    inline void rgb(float r,float g,float b) { rgba(r, g, b, 1.0f); }
    inline void rgba(float r,float g,float b,float a)
    {
        (*_ptr++) = static_cast<unsigned char>(r*255.0);
        (*_ptr++) = static_cast<unsigned char>(g*255.0);
        (*_ptr++) = static_cast<unsigned char>(b*255.0);
        (*_ptr++) = static_cast<unsigned char>(a*255.0);
    }
};

vsg::ref_ptr<osg::Image> formatImage(osg::Image* image, GLenum pixelFormat)
{
    vsg::ref_ptr<osg::Image> new_image = new osg::Image;
    new_image->allocateImage(image->s(), image->t(), image->r(), pixelFormat, GL_UNSIGNED_BYTE);

    // need to copy pixels from image to new_image;
    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            WriteRow operation(new_image->data(0, t, r));
            osg::readRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }

    return new_image;
}

vsg::ImageData readImageFile(vsg::Device* device, vsg::CommandPool* commandPool, VkQueue graphicsQueue, const std::string& filename)
{
    osg::ref_ptr<osg::Image> osg_image = osgDB::readImageFile(filename);
    if (!osg_image)
    {
        return vsg::ImageData();
    }

    if(osg_image->getPixelFormat()!=GL_RGBA || osg_image->getDataType()!=GL_UNSIGNED_BYTE)
    {
        std::cout<<"Reformating osg::Image to GL_RGBA, before = "<<osg_image->getPixelFormat()<<std::endl;
        osg_image = osg2vsg::formatImage(osg_image, GL_RGBA);
        std::cout<<"Reformating osg::Image to GL_RGBA, after = "<<osg_image->getPixelFormat()<<", RGBA="<<GL_RGBA<<std::endl;
    }

    VkDeviceSize imageTotalSize = osg_image->getTotalSizeInBytesIncludingMipmaps();

    vsg::ref_ptr<vsg::Buffer> imageStagingBuffer = vsg::Buffer::create(device, imageTotalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::ref_ptr<vsg::DeviceMemory> imageStagingMemory = vsg::DeviceMemory::create(device, imageStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imageStagingBuffer->bind(imageStagingMemory, 0);

    // copy image data to staging memory
    imageStagingMemory->copy(0, imageTotalSize, osg_image->data());

    std::cout<<"Creating imageStagingBuffer and memorory size = "<<imageTotalSize<<std::endl;


    std::cout<<"VK_FORMAT_R8G8B8A8_UNORM= "<<VK_FORMAT_R8G8B8A8_UNORM<<std::endl;
    std::cout<<"  osg2vsg : "<<osg2vsg::convertGLImageFormatToVulkan(osg_image->getDataType(), osg_image->getPixelFormat())<<std::endl;


    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = osg_image->r()>1 ? VK_IMAGE_TYPE_3D : (osg_image->t()>1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
    imageCreateInfo.extent.width = osg_image->s();
    imageCreateInfo.extent.height = osg_image->t();
    imageCreateInfo.extent.depth = osg_image->r();
    imageCreateInfo.mipLevels = osg_image->getNumMipmapLevels();
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = osg2vsg::convertGLImageFormatToVulkan(osg_image->getDataType(), osg_image->getPixelFormat());
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vsg::ref_ptr<vsg::Image> textureImage = vsg::Image::create(device, imageCreateInfo);
    if (!textureImage)
    {
        return vsg::ImageData();
    }

    vsg::ref_ptr<vsg::DeviceMemory> textureImageDeviceMemory = vsg::DeviceMemory::create(device, textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!textureImageDeviceMemory)
    {
        return vsg::ImageData();
    }

    textureImage->bind(textureImageDeviceMemory, 0);


    vsg::dispatchCommandsToQueue(device, commandPool, graphicsQueue, [&](VkCommandBuffer commandBuffer)
    {

        std::cout<<"Need to dispatch VkCmd's to "<<commandBuffer<<std::endl;

        vsg::ImageMemoryBarrier preCopyImageMemoryBarrier(
                        0, VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        textureImage);

        preCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        std::cout<<"CopyBufferToImage()"<<std::endl;

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(osg_image->s()), static_cast<uint32_t>(osg_image->t()), 1};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        std::cout<<"Post CopyBufferToImage()"<<std::endl;

        vsg::ImageMemoryBarrier postCopyImageMemoryBarrier(
                        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        textureImage);

        postCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        std::cout<<"Post postCopyImageMemoryBarrier()"<<std::endl;
    });

    // clean up staging buffer
    imageStagingBuffer = 0;
    imageStagingMemory = 0;

    // delete osg_image as it's no longer required.
    osg_image = 0;

    vsg::ref_ptr<vsg::Sampler> textureSampler = vsg::Sampler::create(device);
    vsg::ref_ptr<vsg::ImageView> textureImageView = vsg::ImageView::create(device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    std::cout<<"textureSampler = "<<textureSampler.get()<<std::endl;
    std::cout<<"textureImageView = "<<textureImageView.get()<<std::endl;

    return vsg::ImageData(textureSampler, textureImageView, VK_IMAGE_LAYOUT_UNDEFINED);
}


} // end of namespace osg2cpp
