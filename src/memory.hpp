#pragma once
#ifndef MEMORY_HPP_INCLUDED
#define MEMORY_HPP_INCLUDED
#include "vulkan_include.hpp"

namespace vkBasalt
{
    auto findMemoryTypeIndex(const vkroots::VkDeviceDispatch* pDispatch,
                             LogicalDevice*                   pLogicalDevice,
                             uint32_t                         typeFilter,
                             VkMemoryPropertyFlags            properties) -> uint32_t;
}

#endif // MEMORY_HPP_INCLUDED
