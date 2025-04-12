#pragma once
#ifndef IMAGE_VIEW_HPP_INCLUDED
#define IMAGE_VIEW_HPP_INCLUDED
#include <vector>
#include "vulkan_include.hpp"

namespace vkBasalt
{
    auto createImageViews(const vkroots::VkDeviceDispatch* pDispatch,
                          LogicalDevice*                   pLogicalDevice,
                          VkFormat                         format,
                          std::vector<VkImage>             images,
                          VkImageViewType                  viewType   = VK_IMAGE_VIEW_TYPE_2D,
                          VkImageAspectFlags               aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                          uint32_t                         mipLevels  = 1) -> std::vector<VkImageView>;
}

#endif // IMAGE_VIEW_HPP_INCLUDED
