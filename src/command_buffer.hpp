#pragma once
#ifndef COMMAND_BUFFER_HPP_INCLUDED
#define COMMAND_BUFFER_HPP_INCLUDED
#include <vector>
#include <memory>

#include "effect.hpp"

namespace VulkanFX
{

    auto allocateCommandBuffer(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, uint32_t count)
        -> std::vector<VkCommandBuffer>;

    void writeCommandBuffers(const vkroots::VkDeviceDispatch*               pDispatch,
                             LogicalDevice*                                 pLogicalDevice,
                             std::vector<std::shared_ptr<VulkanFX::Effect>> effects,
                             VkImage                                        depthImage,
                             VkImageView                                    depthImageView,
                             VkFormat                                       depthFormat,
                             std::vector<VkCommandBuffer>                   commandBuffers);

    auto createSemaphores(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, uint32_t count) -> std::vector<VkSemaphore>;
} // namespace VulkanFX

#endif // COMMAND_BUFFER_HPP_INCLUDED
