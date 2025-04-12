#pragma once
#ifndef DESCRIPTOR_SET_HPP_INCLUDED
#define DESCRIPTOR_SET_HPP_INCLUDED
#include "vulkan_include.hpp"

namespace vkBasalt
{
    auto createDescriptorPool(const vkroots::VkDeviceDispatch*         pDispatch,
                              LogicalDevice*                           pLogicalDevice,
                              const std::vector<VkDescriptorPoolSize>& poolSizes) -> VkDescriptorPool;

    auto createUniformBufferDescriptorSetLayout(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice) -> VkDescriptorSetLayout;

    auto writeBufferDescriptorSet(const vkroots::VkDeviceDispatch* pDispatch,
                                  LogicalDevice*                   pLogicalDevice,
                                  VkDescriptorPool                 descriptorPool,
                                  VkDescriptorSetLayout            descriptorSetLayout,
                                  VkBuffer                         buffer) -> VkDescriptorSet;

    auto createImageSamplerDescriptorSetLayout(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, uint32_t count)
        -> VkDescriptorSetLayout;

    auto allocateAndWriteImageSamplerDescriptorSets(const vkroots::VkDeviceDispatch*      pDispatch,
                                                    LogicalDevice*                        pLogicalDevice,
                                                    VkDescriptorPool                      descriptorPool,
                                                    VkDescriptorSetLayout                 descriptorSetLayout,
                                                    std::vector<VkSampler>                samplers,
                                                    std::vector<std::vector<VkImageView>> imageViewsVectors) -> std::vector<VkDescriptorSet>;
} // namespace vkBasalt

#endif // DESCRIPTOR_SET_HPP_INCLUDED
