#include "effect_smaa.hpp"

#include <cstring>

#include "image_view.hpp"
#include "descriptor_set.hpp"
#include "renderpass.hpp"
#include "graphics_pipeline.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "sampler.hpp"
#include "image.hpp"
#include "util.hpp"

#include "AreaTex.h"
#include "SearchTex.h"
#include "shader_sources.hpp"

namespace VulkanFX
{
    SmaaEffect::SmaaEffect(const vkroots::VkDeviceDispatch* pDispatch,
                           LogicalDevice*                   pLogicalDevice,
                           VkFormat                         format,
                           VkExtent2D                       imageExtent,
                           std::vector<VkImage>             inputImages,
                           std::vector<VkImage>             outputImages,
                           Config*                          pConfig)
    {
        Logger::debug("in creating SmaaEffect");

        this->pDispatch      = pDispatch;
        this->pLogicalDevice = pLogicalDevice;
        this->format         = format;
        this->imageExtent    = imageExtent;
        this->inputImages    = inputImages;
        this->outputImages   = outputImages;
        this->pConfig        = pConfig;

        // create Images for the first and second pass at once -> less memory fragmentation
        std::vector<VkImage> edgeAndBlendImages = createImages(pDispatch,
                                                               pLogicalDevice,
                                                               inputImages.size() * 2,
                                                               {imageExtent.width, imageExtent.height, 1},
                                                               VK_FORMAT_B8G8R8A8_UNORM, // TODO search for format and save it
                                                               VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                               imageMemory);

        edgeImages  = std::vector<VkImage>(edgeAndBlendImages.begin(), edgeAndBlendImages.begin() + edgeAndBlendImages.size() / 2);
        blendImages = std::vector<VkImage>(edgeAndBlendImages.begin() + edgeAndBlendImages.size() / 2, edgeAndBlendImages.end());

        inputImageViews = createImageViews(pDispatch, pLogicalDevice, format, inputImages);
        Logger::debug("created input ImageViews");
        edgeImageViews = createImageViews(pDispatch, pLogicalDevice, VK_FORMAT_B8G8R8A8_UNORM, edgeImages);
        Logger::debug("created edge  ImageViews");
        blendImageViews = createImageViews(pDispatch, pLogicalDevice, VK_FORMAT_B8G8R8A8_UNORM, blendImages);
        Logger::debug("created blend ImageViews");
        outputImageViews = createImageViews(pDispatch, pLogicalDevice, format, outputImages);
        Logger::debug("created output ImageViews");
        sampler = createSampler(pDispatch, pLogicalDevice);
        Logger::debug("created sampler");

        VkExtent3D areaImageExtent = {AREATEX_WIDTH, AREATEX_HEIGHT, 1};

        areaImage = createImages(pDispatch,
                                 pLogicalDevice,
                                 1,
                                 areaImageExtent,
                                 VK_FORMAT_R8G8_UNORM, // TODO search for format and save it
                                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 areaMemory)[0];

        VkExtent3D searchImageExtent = {SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 1};

        searchImage = createImages(pDispatch,
                                   pLogicalDevice,
                                   1,
                                   searchImageExtent,
                                   VK_FORMAT_R8_UNORM, // TODO search for format and save it
                                   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   searchMemory)[0];

        uploadToImage(pDispatch, pLogicalDevice, areaImage, areaImageExtent, AREATEX_SIZE, areaTexBytes);

        uploadToImage(pDispatch, pLogicalDevice, searchImage, searchImageExtent, SEARCHTEX_SIZE, searchTexBytes);

        areaImageView = createImageViews(pDispatch, pLogicalDevice, VK_FORMAT_R8G8_UNORM, std::vector<VkImage>(1, areaImage))[0];
        Logger::debug("after creating area ImageView");
        searchImageView = createImageViews(pDispatch, pLogicalDevice, VK_FORMAT_R8_UNORM, std::vector<VkImage>(1, searchImage))[0];
        Logger::debug("created search ImageView");

        imageSamplerDescriptorSetLayout = createImageSamplerDescriptorSetLayout(pDispatch, pLogicalDevice, 5);
        Logger::debug("created descriptorSetLayouts");

        VkDescriptorPoolSize imagePoolSize;
        imagePoolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imagePoolSize.descriptorCount = inputImages.size() * 5;

        std::vector<VkDescriptorPoolSize> poolSizes = {imagePoolSize};

        descriptorPool = createDescriptorPool(pDispatch, pLogicalDevice, poolSizes);
        Logger::debug("created descriptorPool");

        // get config options
        struct SmaaOptions
        {
            float   screenWidth;
            float   screenHeight;
            float   reverseScreenWidth;
            float   reverseScreenHeight;
            float   threshold;
            int32_t maxSearchSteps;
            int32_t maxSearchStepsDiag;
            int32_t cornerRounding;
        };

        SmaaOptions smaaOptions;
        smaaOptions.threshold          = pConfig->getOption<float>("smaaThreshold", 0.05f);
        smaaOptions.maxSearchSteps     = pConfig->getOption<int32_t>("smaaMaxSearchSteps", 32);
        smaaOptions.maxSearchStepsDiag = pConfig->getOption<int32_t>("smaaMaxSearchStepsDiag", 16);
        smaaOptions.cornerRounding     = pConfig->getOption<int32_t>("smaaCornerRounding", 25);

        createShaderModule(pDispatch, pLogicalDevice, smaa_edge_vert, &edgeVertexModule);

        bool useColor = pConfig->getOption<std::string>("smaaEdgeDetection", "luma") == "color";

        auto shaderCode = useColor ? smaa_edge_color_frag : smaa_edge_luma_frag;
        createShaderModule(pDispatch, pLogicalDevice, shaderCode, &edgeFragmentModule);

        createShaderModule(pDispatch, pLogicalDevice, smaa_blend_vert, &blendVertexModule);

        createShaderModule(pDispatch, pLogicalDevice, smaa_blend_frag, &blendFragmentModule);

        createShaderModule(pDispatch, pLogicalDevice, smaa_neighbor_vert, &neighborVertexModule);

        createShaderModule(pDispatch, pLogicalDevice, smaa_neighbor_frag, &neignborFragmentModule);

        renderPass      = createRenderPass(pDispatch, pLogicalDevice, format);
        unormRenderPass = createRenderPass(pDispatch, pLogicalDevice, VK_FORMAT_B8G8R8A8_UNORM);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {imageSamplerDescriptorSetLayout};
        pipelineLayout                                          = createGraphicsPipelineLayout(pDispatch, pLogicalDevice, descriptorSetLayouts);

        std::vector<VkSpecializationMapEntry> specMapEntrys(8);
        for (uint32_t i = 0; i < specMapEntrys.size(); i++)
        {
            specMapEntrys[i].constantID = i;
            specMapEntrys[i].offset     = sizeof(float) * i; // TODO not clean to assume that sizeof(int32_t) == sizeof(float)
            specMapEntrys[i].size       = sizeof(float);
        }
        smaaOptions.screenWidth = (float) imageExtent.width, smaaOptions.screenHeight = (float) imageExtent.height,
        smaaOptions.reverseScreenWidth  = 1.0f / imageExtent.width;
        smaaOptions.reverseScreenHeight = 1.0f / imageExtent.height;

        VkSpecializationInfo specializationInfo;
        specializationInfo.mapEntryCount = specMapEntrys.size();
        specializationInfo.pMapEntries   = specMapEntrys.data();
        specializationInfo.dataSize      = sizeof(smaaOptions);
        specializationInfo.pData         = &smaaOptions;

        edgePipeline = createGraphicsPipeline(pDispatch,
                                              pLogicalDevice,
                                              edgeVertexModule,
                                              &specializationInfo,
                                              "main",
                                              edgeFragmentModule,
                                              &specializationInfo,
                                              "main",
                                              imageExtent,
                                              unormRenderPass,
                                              pipelineLayout);

        blendPipeline = createGraphicsPipeline(pDispatch,
                                               pLogicalDevice,
                                               blendVertexModule,
                                               &specializationInfo,
                                               "main",
                                               blendFragmentModule,
                                               &specializationInfo,
                                               "main",
                                               imageExtent,
                                               unormRenderPass,
                                               pipelineLayout);

        neighborPipeline = createGraphicsPipeline(pDispatch,
                                                  pLogicalDevice,
                                                  neighborVertexModule,
                                                  &specializationInfo,
                                                  "main",
                                                  neignborFragmentModule,
                                                  &specializationInfo,
                                                  "main",
                                                  imageExtent,
                                                  renderPass,
                                                  pipelineLayout);

        std::vector<std::vector<VkImageView>> imageViewsVector = {inputImageViews,
                                                                  edgeImageViews,
                                                                  std::vector<VkImageView>(inputImageViews.size(), areaImageView),
                                                                  std::vector<VkImageView>(inputImageViews.size(), searchImageView),
                                                                  blendImageViews};

        imageDescriptorSets = allocateAndWriteImageSamplerDescriptorSets(pDispatch,
                                                                         pLogicalDevice,
                                                                         descriptorPool,
                                                                         imageSamplerDescriptorSetLayout,
                                                                         std::vector<VkSampler>(imageViewsVector.size(), sampler),
                                                                         imageViewsVector);

        edgeFramebuffers     = createFramebuffers(pDispatch, pLogicalDevice, unormRenderPass, imageExtent, {edgeImageViews});
        blendFramebuffers    = createFramebuffers(pDispatch, pLogicalDevice, unormRenderPass, imageExtent, {blendImageViews});
        neignborFramebuffers = createFramebuffers(pDispatch, pLogicalDevice, renderPass, imageExtent, {outputImageViews});
    }
    void SmaaEffect::applyEffect(const vkroots::VkDeviceDispatch* pDispatch, uint32_t imageIndex, VkCommandBuffer commandBuffer)
    {
        Logger::debug("applying smaa effect to cb " + convertToString(commandBuffer));
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
        renderPassBeginInfo.renderPass        = unormRenderPass;
        renderPassBeginInfo.framebuffer       = edgeFramebuffers[imageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = imageExtent;
        VkClearValue clearValue               = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount   = 1;
        renderPassBeginInfo.pClearValues      = &clearValue;
        // edge renderPass
        Logger::debug("before beginn edge renderpass");
        pDispatch->CmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        Logger::debug("after beginn renderpass");

        pDispatch->CmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(imageDescriptorSets[imageIndex]), 0, nullptr);
        Logger::debug("after binding image sampler");

        pDispatch->CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, edgePipeline);
        Logger::debug("after bind pipeliene");

        pDispatch->CmdDraw(commandBuffer, 3, 1, 0, 0);
        Logger::debug("after draw");

        pDispatch->CmdEndRenderPass(commandBuffer);
        Logger::debug("after end renderpass");

        memoryBarrier.image             = edgeImages[imageIndex];
        renderPassBeginInfo.framebuffer = blendFramebuffers[imageIndex];
        // blend renderPass
        pDispatch->CmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        Logger::debug("after the first pipeline barrier");

        Logger::debug("before beginn blend renderpass");
        pDispatch->CmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        Logger::debug("after beginn renderpass");

        pDispatch->CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, blendPipeline);
        Logger::debug("after bind pipeliene");

        pDispatch->CmdDraw(commandBuffer, 3, 1, 0, 0);
        Logger::debug("after draw");

        pDispatch->CmdEndRenderPass(commandBuffer);
        Logger::debug("after end renderpass");

        memoryBarrier.image             = blendImages[imageIndex];
        renderPassBeginInfo.framebuffer = neignborFramebuffers[imageIndex];
        renderPassBeginInfo.renderPass  = renderPass;
        // neighbor renderPass
        pDispatch->CmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        Logger::debug("after the first pipeline barrier");

        Logger::debug("before beginn neighbor renderpass");
        pDispatch->CmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        Logger::debug("after beginn renderpass");

        pDispatch->CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, neighborPipeline);
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
    SmaaEffect::~SmaaEffect()
    {
        Logger::debug("destroying smaa effect " + convertToString(this));
        pDispatch->DestroyPipeline(pLogicalDevice->device, edgePipeline, nullptr);
        pDispatch->DestroyPipeline(pLogicalDevice->device, blendPipeline, nullptr);
        pDispatch->DestroyPipeline(pLogicalDevice->device, neighborPipeline, nullptr);

        pDispatch->DestroyPipelineLayout(pLogicalDevice->device, pipelineLayout, nullptr);
        pDispatch->DestroyRenderPass(pLogicalDevice->device, renderPass, nullptr);
        pDispatch->DestroyRenderPass(pLogicalDevice->device, unormRenderPass, nullptr);
        pDispatch->DestroyDescriptorSetLayout(pLogicalDevice->device, imageSamplerDescriptorSetLayout, nullptr);

        pDispatch->DestroyShaderModule(pLogicalDevice->device, edgeVertexModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, edgeFragmentModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, blendVertexModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, blendFragmentModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, neighborVertexModule, nullptr);
        pDispatch->DestroyShaderModule(pLogicalDevice->device, neignborFragmentModule, nullptr);

        pDispatch->DestroyDescriptorPool(pLogicalDevice->device, descriptorPool, nullptr);
        for (unsigned int i = 0; i < edgeFramebuffers.size(); i++)
        {
            pDispatch->DestroyFramebuffer(pLogicalDevice->device, edgeFramebuffers[i], nullptr);
            pDispatch->DestroyFramebuffer(pLogicalDevice->device, blendFramebuffers[i], nullptr);
            pDispatch->DestroyFramebuffer(pLogicalDevice->device, neignborFramebuffers[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, inputImageViews[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, edgeImageViews[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, blendImageViews[i], nullptr);
            pDispatch->DestroyImageView(pLogicalDevice->device, outputImageViews[i], nullptr);
            vmaDestroyImage(pLogicalDevice->allocator, edgeImages[i], nullptr);
            vmaDestroyImage(pLogicalDevice->allocator, blendImages[i], nullptr);
        }
        Logger::debug("after DestroyImageView");
        pDispatch->DestroyImageView(pLogicalDevice->device, areaImageView, nullptr);
        pDispatch->DestroyImageView(pLogicalDevice->device, searchImageView, nullptr);
        vmaDestroyImage(pLogicalDevice->allocator, areaImage, areaMemory);
        vmaDestroyImage(pLogicalDevice->allocator, searchImage, searchMemory);
        vmaFreeMemory(pLogicalDevice->allocator, imageMemory);

        pDispatch->DestroySampler(pLogicalDevice->device, sampler, nullptr);
    }
} // namespace VulkanFX
