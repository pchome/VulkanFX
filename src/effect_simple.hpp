#pragma once
#ifndef EFFECT_SIMPLE_HPP_INCLUDED
#define EFFECT_SIMPLE_HPP_INCLUDED
#include <vector>
#include "effect.hpp"
#include "config.hpp"

namespace VulkanFX
{
    class SimpleEffect : public Effect
    {
    public:
        SimpleEffect();
        void virtual applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
        virtual ~SimpleEffect();

    protected:
        const vkroots::VkDeviceDispatch* pDispatch;
        LogicalDevice*                   pLogicalDevice;
        std::vector<VkImage>             inputImages;
        std::vector<VkImage>             outputImages;
        std::vector<VkImageView>         inputImageViews;
        std::vector<VkImageView>         outputImageViews;
        std::vector<VkDescriptorSet>     imageDescriptorSets;
        std::vector<VkFramebuffer>       framebuffers;
        VkDescriptorSetLayout            imageSamplerDescriptorSetLayout;
        VkDescriptorPool                 descriptorPool;
        VkShaderModule                   vertexModule;
        VkShaderModule                   fragmentModule;
        VkRenderPass                     renderPass;
        VkPipelineLayout                 pipelineLayout;
        VkPipeline                       graphicsPipeline;
        VkExtent2D                       imageExtent;
        VkFormat                         format;
        VkSampler                        sampler;
        Config*                          pConfig;
        std::vector<uint32_t>            vertexCode;
        std::vector<uint32_t>            fragmentCode;
        VkSpecializationInfo*            pVertexSpecInfo;
        VkSpecializationInfo*            pFragmentSpecInfo;

        // subclasses can put DescriptorSets in here, but the first one will be the input image descriptorSet
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        void init(const vkroots::VkDeviceDispatch* pDispatch,
                  LogicalDevice*                   pLogicalDevice,
                  VkFormat                         format,
                  VkExtent2D                       imageExtent,
                  std::vector<VkImage>             inputImages,
                  std::vector<VkImage>             outputImages,
                  Config*                          pConfig);
    };
} // namespace VulkanFX

#endif // EFFECT_SIMPLE_HPP_INCLUDED
