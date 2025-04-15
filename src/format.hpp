#pragma once
#ifndef FORMAT_HPP_INCLUDED
#define FORMAT_HPP_INCLUDED
#include <vector>
#include "vulkan_include.hpp"

namespace VulkanFX
{
    // Returns a matching sRGB format to a UNORM format if it exist, else returns format
    auto convertToSRGB(VkFormat format) -> VkFormat;
    // Returns a matching UNORM format to a sRGB format if it exist, else returns format
    auto convertToUNORM(VkFormat format) -> VkFormat;
    // Returns true if format is SRGB
    auto isSRGB(VkFormat format) -> bool;
    // Returns true if format is UNORM
    // TODO currently return false if format is UNORM and no matching sRGB format exist
    auto isUNORM(VkFormat format) -> bool;

    auto getSupportedFormat(const vkroots::VkDeviceDispatch* pDispatch,
                            LogicalDevice*                   pLogicalDevice,
                            std::vector<VkFormat>            formats,
                            VkFormatFeatureFlags             features,
                            VkImageTiling                    tiling = VK_IMAGE_TILING_OPTIMAL) -> VkFormat;

    auto getStencilFormat(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice) -> VkFormat;

    auto isDepthFormat(VkFormat format) -> bool;

    auto isStencilFormat(VkFormat format) -> bool;
} // namespace VulkanFX

#endif // FORMAT_HPP_INCLUDED
