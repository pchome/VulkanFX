#pragma once
#ifndef BUFFER_HPP_INCLUDED
#define BUFFER_HPP_INCLUDED
#include "vulkan_include.hpp"

namespace VulkanFX
{
    void createBuffer(const vkroots::VkDeviceDispatch* pDispatch,
                      LogicalDevice*                   pLogicalDevice,
                      VkDeviceSize                     size,
                      VkBufferUsageFlags               usage,
                      VkMemoryPropertyFlags            properties,
                      VkBuffer&                        buffer,
                      VmaAllocation&                   bufferMemory);
}

#endif // BUFFER_HPP_INCLUDED
