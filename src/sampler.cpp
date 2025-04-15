#include "sampler.hpp"

namespace VulkanFX
{
    auto createSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice) -> VkSampler
    {
        VkSampler sampler;

        VkSamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext                   = nullptr;
        samplerCreateInfo.flags                   = 0;
        samplerCreateInfo.magFilter               = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias              = 0.0f;
        samplerCreateInfo.anisotropyEnable        = VK_FALSE;
        samplerCreateInfo.maxAnisotropy           = 16;
        samplerCreateInfo.compareEnable           = VK_FALSE;
        samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod                  = 0.0f;
        samplerCreateInfo.maxLod                  = 0.0f;
        samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        VkResult result = pDispatch->CreateSampler(pLogicalDevice->device, &samplerCreateInfo, nullptr, &sampler);
        ASSERT_VULKAN(result);
        return sampler;
    }

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
    auto createReShadeSampler(const vkroots::VkDeviceDispatch* pDispatch, LogicalDevice* pLogicalDevice, const reshadefx::sampler& samplerInfo)
        -> VkSampler
    {
        VkSampler sampler;

        VkFilter            minFilter;
        VkFilter            magFilter;
        VkSamplerMipmapMode mipmapMode;
        convertReShadeFilter(samplerInfo.filter, minFilter, magFilter, mipmapMode);

        VkSamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext                   = nullptr;
        samplerCreateInfo.flags                   = 0;
        samplerCreateInfo.magFilter               = magFilter;
        samplerCreateInfo.minFilter               = minFilter;
        samplerCreateInfo.mipmapMode              = mipmapMode;
        samplerCreateInfo.addressModeU            = convertReShadeAddressMode(samplerInfo.address_u);
        samplerCreateInfo.addressModeV            = convertReShadeAddressMode(samplerInfo.address_v);
        samplerCreateInfo.addressModeW            = convertReShadeAddressMode(samplerInfo.address_w);
        samplerCreateInfo.mipLodBias              = samplerInfo.lod_bias;
        samplerCreateInfo.anisotropyEnable        = VK_FALSE;
        samplerCreateInfo.maxAnisotropy           = 16;
        samplerCreateInfo.compareEnable           = VK_FALSE;
        samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod                  = samplerInfo.min_lod;
        samplerCreateInfo.maxLod                  = samplerInfo.max_lod;
        samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        VkResult result = pDispatch->CreateSampler(pLogicalDevice->device, &samplerCreateInfo, nullptr, &sampler);
        ASSERT_VULKAN(result);
        return sampler;
    }

    auto convertReShadeAddressMode(const reshadefx::texture_address_mode& addressMode) -> VkSamplerAddressMode
    {
        switch (addressMode)
        {
            case reshadefx::texture_address_mode::wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case reshadefx::texture_address_mode::mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case reshadefx::texture_address_mode::clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case reshadefx::texture_address_mode::border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    void convertReShadeFilter(const reshadefx::filter_mode& textureFilter, VkFilter& minFilter, VkFilter& magFilter, VkSamplerMipmapMode& mipmapMode)
    {
        switch (textureFilter)
        {
            case reshadefx::filter_mode::min_mag_mip_point:
                minFilter  = VK_FILTER_NEAREST;
                magFilter  = VK_FILTER_NEAREST;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                return;
            case reshadefx::filter_mode::min_mag_point_mip_linear:
                minFilter  = VK_FILTER_NEAREST;
                magFilter  = VK_FILTER_NEAREST;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                return;
            case reshadefx::filter_mode::min_point_mag_linear_mip_point:
                minFilter  = VK_FILTER_NEAREST;
                magFilter  = VK_FILTER_LINEAR;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                return;
            case reshadefx::filter_mode::min_point_mag_mip_linear:
                minFilter  = VK_FILTER_NEAREST;
                magFilter  = VK_FILTER_LINEAR;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                return;
            case reshadefx::filter_mode::min_linear_mag_mip_point:
                minFilter  = VK_FILTER_LINEAR;
                magFilter  = VK_FILTER_NEAREST;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                return;
            case reshadefx::filter_mode::min_linear_mag_point_mip_linear:
                minFilter  = VK_FILTER_LINEAR;
                magFilter  = VK_FILTER_NEAREST;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                return;
            case reshadefx::filter_mode::min_mag_linear_mip_point:
                minFilter  = VK_FILTER_LINEAR;
                magFilter  = VK_FILTER_LINEAR;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                return;
            case reshadefx::filter_mode::min_mag_mip_linear:
                minFilter  = VK_FILTER_LINEAR;
                magFilter  = VK_FILTER_LINEAR;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                return;
            case reshadefx::filter_mode::anisotropic:
            default: // (?)
                minFilter  = VK_FILTER_LINEAR;
                magFilter  = VK_FILTER_LINEAR;
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                return;
                // TODO: check (and maybe use) reshade::vulkan::convert_sampler_desc
                // source/vulkan/vulkan_impl_type_convert.hpp
        }
    }
#endif
} // namespace VulkanFX
