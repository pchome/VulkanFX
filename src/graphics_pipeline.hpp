#pragma once
#ifndef GRAPHICS_PIPELINE_HPP_INCLUDED
#define GRAPHICS_PIPELINE_HPP_INCLUDED
#include <vector>
#include <string>
#include <vector>
#include "vulkan_include.hpp"

namespace VulkanFX
{
    auto createGraphicsPipelineLayout(const vkroots::VkDeviceDispatch*   pDispatch,
                                      LogicalDevice*                     pLogicalDevice,
                                      std::vector<VkDescriptorSetLayout> descriptorSetLayouts) -> VkPipelineLayout;

    auto createGraphicsPipeline(const vkroots::VkDeviceDispatch* pDispatch,
                                LogicalDevice*                   pLogicalDevice,
                                VkShaderModule                   vertexModule,
                                VkSpecializationInfo*            vertexSpecializationInfo,
                                std::string                      vertexEntryPoint,
                                VkShaderModule                   fragmentModule,
                                VkSpecializationInfo*            fragmentSpecializationInfo,
                                std::string                      fragmentEntryPoint,
                                VkExtent2D                       extent,
                                VkRenderPass                     renderPass,
                                VkPipelineLayout                 pipelineLayout,
                                bool                             flip = false) -> VkPipeline;

} // namespace VulkanFX

#endif // GRAPHICS_PIPELINE_HPP_INCLUDED
