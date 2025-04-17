#pragma once
#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED
#include <vector>
#include <cstring>
#include "vulkan_include.hpp"

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
                      uint32_t                         mipLevels = 1) -> std::vector<VkImage>;

    void uploadToImage(const vkroots::VkDeviceDispatch* pDispatch,
                       LogicalDevice*                   pLogicalDevice,
                       VkImage                          image,
                       VkExtent3D                       extent,
                       uint32_t                         size,
                       const unsigned char*             writeData,
                       uint32_t                         mipLevels = 1);

    void
    changeImageLayout(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, std::vector<VkImage> images, uint32_t mipLevels = 1);

    void generateMipMaps(const vkroots::VkDeviceDispatch* pDispatch,
                         LogicalDevice*                   pLogicalDevice,
                         VkCommandBuffer                  commandBuffer,
                         VkImage                          image,
                         VkExtent3D                       extent,
                         uint32_t                         mipLevels);
} // namespace VulkanFX

#endif // IMAGE_HPP_INCLUDED
