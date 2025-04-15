#pragma once
#ifndef EFFECT_RESHADE_HPP_INCLUDED
#define EFFECT_RESHADE_HPP_INCLUDED
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "effect.hpp"
#include "config.hpp"
#include "reshade_uniforms.hpp"

namespace VulkanFX
{
    class ReShadeEffect : public Effect
    {
    public:
        ReShadeEffect(const vkroots::VkDeviceDispatch* pDispatch,
                      LogicalDevice*                   pLogicalDevice,
                      VkFormat                         format,
                      VkExtent2D                       imageExtent,
                      std::vector<VkImage>             inputImages,
                      std::vector<VkImage>             outputImages,
                      Config*                          pConfig,
                      std::string                      effectName);
        void virtual applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
        void virtual updateEffect(const vkroots::VkDeviceDispatch* pDispatch) override;
        void virtual useDepthImage(const vkroots::VkDeviceDispatch* pDispatch, VkImageView depthImageView) override;
        virtual ~ReShadeEffect();

    private:
        const vkroots::VkDeviceDispatch* pDispatch;
        LogicalDevice*                   pLogicalDevice;
        std::vector<VkImage>             inputImages;
        std::vector<VkImage>             outputImages;
        std::vector<VkImageView>         inputImageViewsSRGB;
        std::vector<VkImageView>         inputImageViewsUNORM;
        std::vector<VkImageView>         outputImageViewsSRGB;
        std::vector<VkImageView>         outputImageViewsUNORM;

        std::unordered_map<std::string, std::vector<VkImage>>     textureImages;
        std::unordered_map<std::string, std::vector<VkImageView>> textureImageViewsUNORM;
        std::unordered_map<std::string, std::vector<VkImageView>> textureImageViewsSRGB;
        std::unordered_map<std::string, std::vector<VkImageView>> renderImageViewsSRGB;
        std::unordered_map<std::string, std::vector<VkImageView>> renderImageViewsUNORM;

        std::unordered_map<std::string, VkFormat>   textureFormatsUNORM;
        std::unordered_map<std::string, VkFormat>   textureFormatsSRGB;
        std::unordered_map<std::string, uint32_t>   textureMipLevels;
        std::unordered_map<std::string, VkExtent3D> textureExtents;

        std::vector<VkDescriptorSet> inputDescriptorSets;
        std::vector<VkDescriptorSet> outputDescriptorSets;
        std::vector<VkDescriptorSet> backBufferDescriptorSets;

        std::vector<std::vector<VkFramebuffer>> framebuffers;

        VkDescriptorSetLayout                 uniformDescriptorSetLayout;
        VkDescriptorSetLayout                 imageSamplerDescriptorSetLayout;
        VkShaderModule                        shaderModule;
        VkDescriptorPool                      descriptorPool;
        std::vector<VkRenderPass>             renderPasses;
        std::vector<std::vector<std::string>> renderTargets;
        std::vector<VkRenderPassBeginInfo>    renderPassBeginInfos;
        VkPipelineLayout                      pipelineLayout;
        std::vector<VkPipeline>               graphicsPipelines;
        std::vector<bool>                     switchSamplers;
        VkExtent2D                            imageExtent;
        std::vector<VkSampler>                samplers;
        Config*                               pConfig;
        std::string                           effectName;
        reshadefx::effect_module              module;
        std::vector<VkDeviceMemory>           textureMemory;

        VkFormat    inputOutputFormatUNORM;
        VkFormat    inputOutputFormatSRGB;
        VkFormat    stencilFormat;
        VkImage     stencilImage;
        VkImageView stencilImageView;
        // how often the shader writes to the reshade back buffer
        // we need to flip the "backbuffer" after each write if there is a next one
        int                      outputWrites = 0;
        std::vector<VkImage>     backBufferImages;
        std::vector<VkImageView> backBufferImageViewsUNORM;
        std::vector<VkImageView> backBufferImageViewsSRGB;
        VkBuffer                 stagingBuffer;
        VkDeviceMemory           stagingBufferMemory;
        uint32_t                 bufferSize;
        VkDescriptorSet          bufferDescriptorSet;

        std::vector<std::shared_ptr<ReShadeUniform>> uniforms;

        void createReShadeModule();
        auto convertReShadeFormat(reshadefx::texture_format texFormat) -> VkFormat;
        auto convertReShadeCompareOp(reshadefx::stencil_func compareOp) -> VkCompareOp;
        auto convertReShadeStencilOp(reshadefx::stencil_op stencilOp) -> VkStencilOp;
        auto convertReShadeBlendOp(reshadefx::blend_op blendOp) -> VkBlendOp;
        auto convertReShadeBlendFactor(reshadefx::blend_factor blendFactor) -> VkBlendFactor;
    };
} // namespace VulkanFX

#endif // EFFECT_RESHADE_HPP_INCLUDED
