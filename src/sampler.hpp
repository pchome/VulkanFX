#pragma once
#ifndef SAMPLER_HPP_INCLUDED
#define SAMPLER_HPP_INCLUDED
#include "vulkan_include.hpp"

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
#include "reshade/effect_module.hpp"
#endif

namespace vkBasalt
{
    auto createSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice) -> VkSampler;

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
    auto createReshadeSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, const reshadefx::sampler& samplerInfo)
        -> VkSampler;

    auto convertReshadeAddressMode(const reshadefx::texture_address_mode& addressMode) -> VkSamplerAddressMode;

    void convertReshadeFilter(const reshadefx::filter_mode& textureFilter, VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode);
#endif

} // namespace vkBasalt

#endif // SAMPLER_HPP_INCLUDED
