#pragma once
#ifndef RENDERPASS_HPP_INCLUDED
#define RENDERPASS_HPP_INCLUDED
#include "vulkan_include.hpp"

namespace VulkanFX
{
    auto createRenderPass(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, VkFormat format) -> VkRenderPass;
}

#endif // RENDERPASS_HPP_INCLUDED
