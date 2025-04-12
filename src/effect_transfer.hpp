#pragma once
#ifndef EFFECT_TRANSFER_HPP_INCLUDED
#define EFFECT_TRANSFER_HPP_INCLUDED
#include "vulkan_include.hpp"
#include "effect.hpp"
#include "config.hpp"

namespace vkBasalt
{
    class TransferEffect : public Effect
    {
    public:
        TransferEffect(const vkroots::VkDeviceDispatch* pDispatch,
                       LogicalDevice*                   pLogicalDevice,
                       VkFormat                         format,
                       VkExtent2D                       imageExtent,
                       std::vector<VkImage>             inputImages,
                       std::vector<VkImage>             outputImages,
                       Config*                          pConfig);
        void virtual applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
        virtual ~TransferEffect();

    private:
        LogicalDevice*       pLogicalDevice;
        std::vector<VkImage> inputImages;
        std::vector<VkImage> outputImages;
        VkExtent2D           imageExtent;
        VkFormat             format;
        Config*              pConfig;
    };
} // namespace vkBasalt
#endif // EFFECT_TRANSFER_HPP_INCLUDED
