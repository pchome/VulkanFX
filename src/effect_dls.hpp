#pragma once
#ifndef EFFECT_DLS_HPP_INCLUDED
#define EFFECT_DLS_HPP_INCLUDED
#include <vector>
#include "effect_simple.hpp"
#include "config.hpp"

namespace VulkanFX
{
    class DlsEffect : public SimpleEffect
    {
    public:
        DlsEffect(const vkroots::VkDeviceDispatch* pDispatch,
                  LogicalDevice*                   pLogicalDevice,
                  VkFormat                         format,
                  VkExtent2D                       imageExtent,
                  std::vector<VkImage>             inputImages,
                  std::vector<VkImage>             outputImages,
                  Config*                          pConfig);
        ~DlsEffect();
    };
} // namespace VulkanFX

#endif // EFFECT_DLS_HPP_INCLUDED
