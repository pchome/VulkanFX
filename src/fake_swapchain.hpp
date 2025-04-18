#pragma once
#ifndef FAKE_SWAPCHAIN_HPP_INCLUDED
#define FAKE_SWAPCHAIN_HPP_INCLUDED
#include <vector>
#include "vulkan_include.hpp"

namespace VulkanFX
{
    auto createFakeSwapchainImages(const vkroots::VkDeviceDispatch* pDispatch,
                                   LogicalDevice*                   pLogicalDevice,
                                   VkSwapchainCreateInfoKHR         swapchainCreateInfo,
                                   uint32_t                         count,
                                   VmaAllocation&                   deviceMemory) -> std::vector<VkImage>;
}

#endif // FAKE_SWAPCHAIN_HPP_INCLUDED
