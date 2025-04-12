#pragma once
#ifndef FAKE_SWAPCHAIN_HPP_INCLUDED
#define FAKE_SWAPCHAIN_HPP_INCLUDED
#include <vector>
#include "vulkan_include.hpp"

namespace vkBasalt
{
    auto createFakeSwapchainImages(const vkroots::VkDeviceDispatch* pDispatch,
                                   LogicalDevice*                   pLogicalDevice,
                                   VkSwapchainCreateInfoKHR         swapchainCreateInfo,
                                   uint32_t                         count,
                                   VkDeviceMemory&                  deviceMemory) -> std::vector<VkImage>;
}

#endif // FAKE_SWAPCHAIN_HPP_INCLUDED
