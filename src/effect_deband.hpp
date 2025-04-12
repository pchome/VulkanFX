#pragma once
#ifndef EFFECT_DEBAND_HPP_INCLUDED
#define EFFECT_DEBAND_HPP_INCLUDED
#include <vector>
#include "effect_simple.hpp"
#include "config.hpp"

namespace vkBasalt
{
    class DebandEffect : public SimpleEffect
    {
    public:
        DebandEffect(const vkroots::VkDeviceDispatch* pDispatch,
                     LogicalDevice*                   pLogicalDevice,
                     VkFormat                         format,
                     VkExtent2D                       imageExtent,
                     std::vector<VkImage>             inputImages,
                     std::vector<VkImage>             outputImages,
                     Config*                          pConfig);
        ~DebandEffect();
    };
} // namespace vkBasalt

#endif // EFFECT_DEBAND_HPP_INCLUDED
