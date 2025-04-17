#include "image.hpp"
#include "buffer.hpp"
#include "format.hpp"

namespace VulkanFX
{
    auto createImages(const vkroots::VkDeviceDispatch* pDispatch,
                      LogicalDevice*                   pLogicalDevice,
                      uint32_t                         count,
                      VkExtent3D                       extent,
                      VkFormat                         format,
                      VkImageUsageFlags                usage,
                      VkMemoryPropertyFlags            properties,
                      VmaAllocation&                   imageMemory,
                      uint32_t                         mipLevels) -> std::vector<VkImage>
    {
        std::vector<VkImage> images(count);

        VkFormat srgbFormat  = isSRGB(format) ? format : convertToSRGB(format);
        VkFormat unormFormat = isSRGB(format) ? convertToUNORM(format) : format;

        VkFormat formats[] = {unormFormat, srgbFormat};

        VkImageFormatListCreateInfoKHR imageFormatListCreateInfo;
        imageFormatListCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
        imageFormatListCreateInfo.pNext           = nullptr;
        imageFormatListCreateInfo.viewFormatCount = 2;
        imageFormatListCreateInfo.pViewFormats    = formats;

        VkImageCreateInfo imageCreateInfo;
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = (unormFormat == srgbFormat) ? nullptr : &imageFormatListCreateInfo;
        imageCreateInfo.flags = (unormFormat == srgbFormat) ? 0 : VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        if (extent.depth == 1)
        {
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        }
        else
        {
            imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
        }
        imageCreateInfo.format                = format;
        imageCreateInfo.extent                = extent;
        imageCreateInfo.mipLevels             = mipLevels;
        imageCreateInfo.arrayLayers           = 1;
        imageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage                 = usage;
        imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.queueFamilyIndexCount = 0;       // Don't care
        imageCreateInfo.pQueueFamilyIndices   = nullptr; // Don't care
        imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

        VkResult result;
        VmaAllocationCreateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        memoryAllocateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        memoryAllocateInfo.preferredFlags = properties;

        for (uint32_t i = 0; i < count; i++)
        {
            result = vmaCreateImage(pLogicalDevice->allocator, &imageCreateInfo, &memoryAllocateInfo, &(images[i]), &imageMemory, nullptr);
            ASSERT_VULKAN(result);
        }
        return images;
    }

    void uploadToImage(const vkroots::VkDeviceDispatch* pDispatch,
                       LogicalDevice*                   pLogicalDevice,
                       VkImage                          image,
                       VkExtent3D                       extent,
                       uint32_t                         size,
                       const unsigned char*             writeData,
                       uint32_t                         mipLevels)
    {

        VkBuffer       stagingBuffer;
        VmaAllocation stagingMemory;

        createBuffer(pDispatch,
                     pLogicalDevice,
                     size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingMemory);
        void*    data = nullptr;
        VkResult result = vmaMapMemory(pLogicalDevice->allocator, stagingMemory, &data);
        ASSERT_VULKAN(result);
        std::memcpy(data, writeData, size);
        vmaUnmapMemory(pLogicalDevice->allocator, stagingMemory);

        VkCommandBufferAllocateInfo allocInfo = {};

        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = pLogicalDevice->commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        pDispatch->AllocateCommandBuffers(pLogicalDevice->device, &allocInfo, &commandBuffer);
        // initialize dispatch table for commandBuffer since it is a dispatchable object
        initializeDispatchTable(commandBuffer, pLogicalDevice->device);

        VkCommandBufferBeginInfo beginInfo = {};

        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        pDispatch->BeginCommandBuffer(commandBuffer, &beginInfo);

        VkImageMemoryBarrier memoryBarrier;
        memoryBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext                           = nullptr;
        memoryBarrier.srcAccessMask                   = 0;
        memoryBarrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        memoryBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.image                           = image;
        memoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        memoryBarrier.subresourceRange.baseMipLevel   = 0;
        memoryBarrier.subresourceRange.levelCount     = 1;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount     = 1;

        pDispatch->CmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

        VkBufferImageCopy region;
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = extent;

        pDispatch->CmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        pDispatch->CmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

        generateMipMaps(pDispatch, pLogicalDevice, commandBuffer, image, extent, mipLevels);

        pDispatch->EndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};

        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        pDispatch->QueueSubmit(pLogicalDevice->queue, 1, &submitInfo, VK_NULL_HANDLE);
        pDispatch->QueueWaitIdle(pLogicalDevice->queue);

        pDispatch->FreeCommandBuffers(pLogicalDevice->device, pLogicalDevice->commandPool, 1, &commandBuffer);
        vmaDestroyBuffer(pLogicalDevice->allocator, stagingBuffer, stagingMemory);
    }

    void changeImageLayout(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, std::vector<VkImage> images, uint32_t mipLevels)
    {
        VkCommandBufferAllocateInfo allocInfo = {};

        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = pLogicalDevice->commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        pDispatch->AllocateCommandBuffers(pLogicalDevice->device, &allocInfo, &commandBuffer);
        // initialize dispatch table for commandBuffer since it is a dispatchable object
        initializeDispatchTable(commandBuffer, pLogicalDevice->device);

        VkCommandBufferBeginInfo beginInfo = {};

        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        pDispatch->BeginCommandBuffer(commandBuffer, &beginInfo);

        VkImageMemoryBarrier memoryBarrier;
        memoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext               = nullptr;
        memoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        memoryBarrier.srcAccessMask       = 0;
        memoryBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        memoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        memoryBarrier.subresourceRange.baseMipLevel   = 0;
        memoryBarrier.subresourceRange.levelCount     = mipLevels;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount     = 1;

        for (auto& image : images)
        {
            memoryBarrier.image = image;
            pDispatch->CmdPipelineBarrier(
                commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        }

        pDispatch->EndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        pDispatch->QueueSubmit(pLogicalDevice->queue, 1, &submitInfo, VK_NULL_HANDLE);
        pDispatch->QueueWaitIdle(pLogicalDevice->queue);

        pDispatch->FreeCommandBuffers(pLogicalDevice->device, pLogicalDevice->commandPool, 1, &commandBuffer);
    }

    void generateMipMaps(const vkroots::VkDeviceDispatch* pDispatch,
                         LogicalDevice*                   pLogicalDevice,
                         VkCommandBuffer                  commandBuffer,
                         VkImage                          image,
                         VkExtent3D                       extent,
                         uint32_t                         mipLevels)
    {
        if (mipLevels < 2)
        {
            return;
        }
        int32_t width  = extent.width;
        int32_t height = extent.height;
        int32_t depth  = extent.depth;

        VkImageMemoryBarrier memoryBarrier;
        memoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext               = nullptr;
        memoryBarrier.image               = image;
        memoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        memoryBarrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        memoryBarrier.srcAccessMask       = 0;
        memoryBarrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
        memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        memoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        memoryBarrier.subresourceRange.baseMipLevel   = 0;
        memoryBarrier.subresourceRange.levelCount     = 1;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount     = 1;

        for (uint32_t i = 1; i < mipLevels; i++)
        {
            VkImageBlit imageBlit;

            imageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.mipLevel       = i - 1;
            imageBlit.srcSubresource.baseArrayLayer = 0;
            imageBlit.srcSubresource.layerCount     = 1;
            imageBlit.srcOffsets[0]                 = {0, 0, 0};
            imageBlit.srcOffsets[1]                 = {width, height, depth};

            width  = (width == 1) ? 1 : width / 2;
            height = (height == 1) ? 1 : height / 2;
            depth  = (depth == 1) ? 1 : depth / 2;

            imageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.mipLevel       = i;
            imageBlit.dstSubresource.baseArrayLayer = 0;
            imageBlit.dstSubresource.layerCount     = 1;
            imageBlit.dstOffsets[0]                 = {0, 0, 0};
            imageBlit.dstOffsets[1]                 = {width, height, depth};

            memoryBarrier.subresourceRange.baseMipLevel = i - 1;

            memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            memoryBarrier.srcAccessMask = 0;
            memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            pDispatch->CmdPipelineBarrier(
                commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

            memoryBarrier.subresourceRange.baseMipLevel = i;

            memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
            memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.srcAccessMask = 0;
            memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            pDispatch->CmdPipelineBarrier(
                commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

            pDispatch->CmdBlitImage(commandBuffer,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    1,
                                    &imageBlit,
                                    VK_FILTER_LINEAR);

            memoryBarrier.subresourceRange.baseMipLevel = i - 1;

            memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            pDispatch->CmdPipelineBarrier(
                commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

            memoryBarrier.subresourceRange.baseMipLevel = i;

            memoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            pDispatch->CmdPipelineBarrier(
                commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        }
    }
} // namespace VulkanFX
