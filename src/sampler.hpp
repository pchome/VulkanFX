#ifndef SAMPLER_HPP_INCLUDED
#define SAMPLER_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "vulkan_include.hpp"

#include "logical_device.hpp"
#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
#include "reshade/effect_module.hpp"
#endif

namespace vkBasalt
{
    VkSampler createSampler(LogicalDevice* pLogicalDevice);
#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
    VkSampler createReshadeSampler(LogicalDevice* pLogicalDevice, const reshadefx::sampler& samplerInfo);

    VkSamplerAddressMode convertReshadeAddressMode(const reshadefx::texture_address_mode& addressMode);

    void
    convertReshadeFilter(const reshadefx::filter_mode& textureFilter, VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode);
#endif
} // namespace vkBasalt

#endif // SAMPLER_HPP_INCLUDED
