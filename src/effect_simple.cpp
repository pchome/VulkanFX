#include "effect_simple.hpp"

#include <cstring>

#include "image_view.hpp"
#include "descriptor_set.hpp"
#include "renderpass.hpp"
#include "graphics_pipeline.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "sampler.hpp"
#include "util.hpp"

namespace vkBasalt
{
    SimpleEffect::SimpleEffect()
    {
    }
    void SimpleEffect::init(const vkroots::VkDeviceDispatch* pDispatch,
                            LogicalDevice*                   pLogicalDevice,
                            VkFormat                         format,
                            VkExtent2D                       imageExtent,
                            std::vector<VkImage>             inputImages,
                            std::vector<VkImage>             outputImages,
                            Config*                          pConfig)
    {
        Logger::debug("in creating SimpleEffect");

        this->pDispatch      = pDispatch;
        this->pLogicalDevice = pLogicalDevice;
        this->format         = format;
        this->imageExtent    = imageExtent;
        this->inputImages    = inputImages;
        this->outputImages   = outputImages;
        this->pConfig        = pConfig;

        inputImageViews = createImageViews(pDispatch, pLogicalDevice, format, inputImages);
        Logger::debug("created input ImageViews");
        outputImageViews = createImageViews(pDispatch, pLogicalDevice, format, outputImages);
        Logger::debug("created ImageViews");
        sampler = createSampler(pDispatch, pLogicalDevice);
        Logger::debug("created sampler");

        imageSamplerDescriptorSetLayout = createImageSamplerDescriptorSetLayout(pDispatch, pLogicalDevice, 1);
        Logger::debug("created descriptorSetLayouts");

        VkDescriptorPoolSize imagePoolSize;
        imagePoolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imagePoolSize.descriptorCount = inputImages.size() + 10;

        std::vector<VkDescriptorPoolSize> poolSizes = {imagePoolSize};

        descriptorPool = createDescriptorPool(pDispatch, pLogicalDevice, poolSizes);
        Logger::debug("created descriptorPool");

        createShaderModule(pDispatch, pLogicalDevice, vertexCode, &vertexModule);
        createShaderModule(pDispatch, pLogicalDevice, fragmentCode, &fragmentModule);

        renderPass = createRenderPass(pDispatch, pLogicalDevice, format);

        descriptorSetLayouts.insert(descriptorSetLayouts.begin(), imageSamplerDescriptorSetLayout);
        pipelineLayout = createGraphicsPipelineLayout(pDispatch, pLogicalDevice, descriptorSetLayouts);

        graphicsPipeline = createGraphicsPipeline(pDispatch,
                                                  pLogicalDevice,
                                                  vertexModule,
                                                  pVertexSpecInfo,
                                                  "main",
                                                  fragmentModule,
                                                  pFragmentSpecInfo,
                                                  "main",
                                                  imageExtent,
                                                  renderPass,
                                                  pipelineLayout);

        imageDescriptorSets = allocateAndWriteImageSamplerDescriptorSets(pDispatch,
                                                                         pLogicalDevice,
                                                                         descriptorPool,
                                                                         imageSamplerDescriptorSetLayout,
                                                                         {sampler},
                                                                         std::vector<std::vector<VkImageView>>(1, inputImageViews));

        framebuffers = createFramebuffers(pDispatch, pLogicalDevice, renderPass, imageExtent, {outputImageViews});
    }
    void SimpleEffect::applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer)
    {
        Logger::debug("applying SimpleEffect to cb " + convertToString(commandBuffer));
        // Used to make the Image accessable by the shader
        VkImageMemoryBarrier memoryBarrier;
        memoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext               = nullptr;
        memoryBarrier.srcAccessMask       = VK_ACCESS_MEMORY_WRITE_BIT;
        memoryBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        memoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memoryBarrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.image               = inputImages[imageIndex];

        memoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        memoryBarrier.subresourceRange.baseMipLevel   = 0;
        memoryBarrier.subresourceRange.levelCount     = 1;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount     = 1;

        // Reverses the first Barrier
        VkImageMemoryBarrier secondBarrier;
        secondBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        secondBarrier.pNext               = nullptr;
        secondBarrier.srcAccessMask       = VK_ACCESS_SHADER_READ_BIT;
        secondBarrier.dstAccessMask       = 0;
        secondBarrier.oldLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        secondBarrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        secondBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        secondBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        secondBarrier.image               = inputImages[imageIndex];

        secondBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        secondBarrier.subresourceRange.baseMipLevel   = 0;
        secondBarrier.subresourceRange.levelCount     = 1;
        secondBarrier.subresourceRange.baseArrayLayer = 0;
        secondBarrier.subresourceRange.layerCount     = 1;

        pDispatch->CmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        Logger::debug("after the first pipeline barrier");

        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext             = nullptr;
        renderPassBeginInfo.renderPass        = renderPass;
        renderPassBeginInfo.framebuffer       = framebuffers[imageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = imageExtent;
        VkClearValue clearValue               = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount   = 1;
        renderPassBeginInfo.pClearValues      = &clearValue;

        Logger::debug("before beginn renderpass");
        pDispatch->CmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        Logger::debug("after beginn renderpass");

        pDispatch->CmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(imageDescriptorSets[imageIndex]), 0, nullptr);
        Logger::debug("after binding image sampler");

        pDispatch->CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        Logger::debug("after bind pipeliene");

        pDispatch->CmdDraw(commandBuffer, 3, 1, 0, 0);
        Logger::debug("after draw");

        pDispatch->CmdEndRenderPass(commandBuffer);
        Logger::debug("after end renderpass");

        pDispatch->CmdPipelineBarrier(commandBuffer,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      0,
                                      0,
                                      nullptr,
                                      0,
                                      nullptr,
                                      1,
                                      &secondBarrier);
        Logger::debug("after the second pipeline barrier");
    }
    SimpleEffect::~SimpleEffect()
    {
        Logger::debug("destroying SimpleEffect " + convertToString(this));
        pDispatch->DestroyPipeline(pLogicalDevice->device, graphicsPipeline, nullptr);
        pDispatch->DestroyPipelineLayout(pLogicalDevice->device, pipelineLayout, nullptr);
        pDispatch->DestroyRenderPass(pLogicalDevice->device, renderPass, nullptr);
        pDispatch->DestroyDescriptorSetLayout(pLogicalDevice->device, imageSamplerDescriptorSetLayout, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, vertexModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, fragmentModule, nullptr);

        pDispatch->DestroyDescriptorPool(pLogicalDevice->device, descriptorPool, nullptr);
        for (unsigned int i = 0; i < framebuffers.size(); i++)
        {
            pDispatch->DestroyFramebuffer(pLogicalDevice->device, framebuffers[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, inputImageViews[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, outputImageViews[i], nullptr);
        }
        Logger::debug("after DestroyImageView");
        pDispatch->DestroySampler(pLogicalDevice->device, sampler, nullptr);
    }
} // namespace vkBasalt
