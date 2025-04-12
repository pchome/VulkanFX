#pragma once
#ifndef EFFECT_FXAA_HPP_INCLUDED
#define EFFECT_FXAA_HPP_INCLUDED
#include <vector>
#include "effect_simple.hpp"
#include "config.hpp"

namespace vkBasalt
{
    class FxaaEffect : public SimpleEffect
    {
    public:
        FxaaEffect(const vkroots::VkDeviceDispatch* pDispatch,
                   LogicalDevice*                   pLogicalDevice,
                   VkFormat                         format,
                   VkExtent2D                       imageExtent,
                   std::vector<VkImage>             inputImages,
                   std::vector<VkImage>             outputImages,
                   Config*                          pConfig);
        ~FxaaEffect();
    };
} // namespace vkBasalt

#endif // EFFECT_FXAA_HPP_INCLUDED
