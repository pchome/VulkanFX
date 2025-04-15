#pragma once
#ifndef FRAMEBUFFER_HPP_INCLUDED
#define FRAMEBUFFER_HPP_INCLUDED
#include <vector>
#include "vulkan_include.hpp"

namespace VulkanFX
{
    auto createFramebuffers(const vkroots::VkDeviceDispatch*      pDispatch,
                            LogicalDevice*                        pLogicalDevice,
                            VkRenderPass                          renderPass,
                            VkExtent2D&                           extent,
                            std::vector<std::vector<VkImageView>> imageViews) -> std::vector<VkFramebuffer>;
}

#endif // FRAMEBUFFER_HPP_INCLUDED
