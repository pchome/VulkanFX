#pragma once
#ifndef SAMPLER_HPP_INCLUDED
#define SAMPLER_HPP_INCLUDED
#include "vulkan_include.hpp"

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
#include "reshade/effect_module.hpp"
#endif

namespace VulkanFX
{
    auto createSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice) -> VkSampler;

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
    auto createReShadeSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, const reshadefx::sampler& samplerInfo)
        -> VkSampler;

    auto convertReShadeAddressMode(const reshadefx::texture_address_mode& addressMode) -> VkSamplerAddressMode;

    void convertReShadeFilter(const reshadefx::filter_mode& textureFilter, VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode);
#endif

} // namespace VulkanFX

#endif // SAMPLER_HPP_INCLUDED
