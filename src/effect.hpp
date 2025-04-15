#pragma once
#ifndef EFFECT_HPP_INCLUDED
#define EFFECT_HPP_INCLUDED
#include "vulkan_include.hpp"

namespace VulkanFX
{
    class Effect
    {
    public:
        void virtual applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer) = 0;
        void virtual updateEffect(const vkroots::VkDeviceDispatch* pDispatch){};
        void virtual useDepthImage(const vkroots::VkDeviceDispatch* pDispatch, VkImageView depthImageView){};
        virtual ~Effect() {};

    private:
    };
} // namespace VulkanFX

#endif // EFFECT_HPP_INCLUDED
