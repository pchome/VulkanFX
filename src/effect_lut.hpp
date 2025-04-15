#pragma once
#ifndef EFFECT_LUT_HPP_INCLUDED
#define EFFECT_LUT_HPP_INCLUDED
#include <vector>
#include "effect_simple.hpp"
#include "config.hpp"

namespace VulkanFX
{
    class LutEffect : public SimpleEffect
    {
    public:
        LutEffect(const vkroots::VkDeviceDispatch* pDispatch,
                  LogicalDevice*                   pLogicalDevice,
                  VkFormat                         format,
                  VkExtent2D                       imageExtent,
                  std::vector<VkImage>             inputImages,
                  std::vector<VkImage>             outputImages,
                  Config*                          pConfig);
        ~LutEffect();
        void applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer) override;

    private:
        VkImage               lutImage;
        VkDeviceMemory        lutMemory;
        VkImageView           lutImageView;
        VkDescriptorSetLayout lutDescriptorSetLayout;
        VkDescriptorPool      lutDescriptorPool;
        VkDescriptorSet       lutDescriptorSet;
    };
} // namespace VulkanFX

#endif // EFFECT_LUT_HPP_INCLUDED
