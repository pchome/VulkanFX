#pragma once
#ifndef EFFECT_CAS_HPP_INCLUDED
#define EFFECT_CAS_HPP_INCLUDED
#include <vector>
#include "effect_simple.hpp"
#include "config.hpp"

namespace VulkanFX
{
    class CasEffect : public SimpleEffect
    {
    public:
        CasEffect(const vkroots::VkDeviceDispatch* pDispatch,
                  LogicalDevice*                   pLogicalDevice,
                  VkFormat                         format,
                  VkExtent2D                       imageExtent,
                  std::vector<VkImage>             inputImages,
                  std::vector<VkImage>             outputImages,
                  Config*                          pConfig);
        ~CasEffect();
    };
} // namespace VulkanFX

#endif // EFFECT_CAS_HPP_INCLUDED
