#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include <cstring>

#include "vulkan_include.hpp"
#include "vkroots_hack.hpp"

#include "util.hpp"
#include "keyboard_input.hpp"

#include "logical_swapchain.hpp"

#include "image_view.hpp"

#include "command_buffer.hpp"

#include "config.hpp"
#include "fake_swapchain.hpp"
#include "format.hpp"
#include "logger.hpp"

#include "effect.hpp"
#include "effect_fxaa.hpp"
#include "effect_cas.hpp"
#include "effect_dls.hpp"
#include "effect_smaa.hpp"
#include "effect_deband.hpp"
#include "effect_lut.hpp"

#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
#include "effect_reshade.hpp"
#endif

#include "effect_transfer.hpp"

#if defined(__GNUC__) && __GNUC__ >= 4
#define VK_BASALT_EXPORT __attribute__((visibility("default")))
#else
#error "Unsupported platform!"
#endif

namespace
{
    std::shared_ptr<vkBasalt::Config> pConfig = nullptr;
    std::mutex                        globalLock;
} // namespace

namespace vkBasalt
{
    Logger Logger::s_instance;

#ifdef _GCC_
    using scoped_lock __attribute__((unused)) = std::lock_guard<std::mutex>;
#else
    using scoped_lock = std::lock_guard<std::mutex>;
#endif

    using deviceMapData    = LogicalDevice;
    using swapchainMapData = LogicalSwapchain;

    // VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE(deviceMap, void*)
    // VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE(swapchainMap, VkSwapchainKHR)
    VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE_EXT(deviceMap, void*)
    VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE_EXT(swapchainMap, VkSwapchainKHR)

    class VkInstanceOverrides
    {
    public:
        static auto CreateDevice(const vkroots::VkInstanceDispatch* pDispatch,
                                 VkPhysicalDevice                   physicalDevice,
                                 const VkDeviceCreateInfo*          pCreateInfo,
                                 const VkAllocationCallbacks*       pAllocator,
                                 VkDevice*                          pDevice)
        {
            scoped_lock l(globalLock);
            Logger::trace("vkCreateDevice");

            // check and activate extentions
            uint32_t extensionCount = 0;
            pDispatch->EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            pDispatch->EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionProperties.data());

            bool supportsMutableFormat = false;
            for (VkExtensionProperties properties : extensionProperties)
            {
                if (properties.extensionName == std::string("VK_KHR_swapchain_mutable_format"))
                {
                    Logger::debug("device supports VK_KHR_swapchain_mutable_format");
                    supportsMutableFormat = true;
                    break;
                }
            }

            VkPhysicalDeviceProperties deviceProps;
            pDispatch->GetPhysicalDeviceProperties(physicalDevice, &deviceProps);

            VkDeviceCreateInfo       modifiedCreateInfo = *pCreateInfo;
            std::vector<const char*> enabledExtensionNames;
            if (modifiedCreateInfo.enabledExtensionCount)
            {
                enabledExtensionNames =
                    std::vector<const char*>(modifiedCreateInfo.ppEnabledExtensionNames,
                                             modifiedCreateInfo.ppEnabledExtensionNames + modifiedCreateInfo.enabledExtensionCount);
            }

            if (supportsMutableFormat)
            {
                Logger::debug("activating mutable_format");
                addUniqueCString(enabledExtensionNames, "VK_KHR_swapchain_mutable_format");
            }
            // if (deviceProps.apiVersion < VK_API_VERSION_1_2 || instanceVersionMap[GetKey(physicalDevice)] < VK_API_VERSION_1_2)
            if (deviceProps.apiVersion < VK_API_VERSION_1_2)
            {
                addUniqueCString(enabledExtensionNames, "VK_KHR_image_format_list");
            }
            modifiedCreateInfo.ppEnabledExtensionNames = enabledExtensionNames.data();
            modifiedCreateInfo.enabledExtensionCount   = enabledExtensionNames.size();

            // Active needed Features
            VkPhysicalDeviceFeatures deviceFeatures = {};
            if (modifiedCreateInfo.pEnabledFeatures)
            {
                deviceFeatures = *(modifiedCreateInfo.pEnabledFeatures);
            }
            deviceFeatures.shaderImageGatherExtended = VK_TRUE;
            modifiedCreateInfo.pEnabledFeatures      = &deviceFeatures;

            VkResult ret = pDispatch->CreateDevice(physicalDevice, &modifiedCreateInfo, pAllocator, pDevice);

            if (ret != VK_SUCCESS)
                return ret;

            auto pLogicalDevice = deviceMap::create(*pDevice,
                                                    deviceMapData{
                                                        .device                = *pDevice,
                                                        .physicalDevice        = physicalDevice,
                                                        .instance              = pDispatch->Instance,
                                                        .queue                 = VK_NULL_HANDLE,
                                                        .queueFamilyIndex      = 0,
                                                        .commandPool           = VK_NULL_HANDLE,
                                                        .supportsMutableFormat = supportsMutableFormat,
                                                    });

            return VK_SUCCESS;
        }
    };

    class VkDeviceOverrides
    {
    public:
        static auto DestroyDevice(const vkroots::VkDeviceDispatch* pDispatch, VkDevice device, const VkAllocationCallbacks* pAllocator)
        {
            if (!device)
                return;

            scoped_lock l(globalLock);

            Logger::trace("vkDestroyDevice");

            auto pLogicalDevice = deviceMap::get(device);
            if (pLogicalDevice->commandPool != VK_NULL_HANDLE)
            {
                Logger::debug("DestroyCommandPool");
                pDispatch->DestroyCommandPool(device, pLogicalDevice->commandPool, pAllocator);
            }

            pDispatch->DestroyDevice(device, pAllocator);

            deviceMap::remove(device);
        }

        static auto CreateSwapchainKHR(const vkroots::VkDeviceDispatch* pDispatch,
                                       VkDevice                         device,
                                       const VkSwapchainCreateInfoKHR*  pCreateInfo,
                                       const VkAllocationCallbacks*     pAllocator,
                                       VkSwapchainKHR*                  pSwapchain)
        {
            scoped_lock l(globalLock);

            Logger::trace("vkCreateSwapchainKHR");

            auto pLogicalDevice = deviceMap::get(device);

            VkSwapchainCreateInfoKHR modifiedCreateInfo = *pCreateInfo;

            VkFormat format = modifiedCreateInfo.imageFormat;

            VkFormat srgbFormat  = isSRGB(format) ? format : convertToSRGB(format);
            VkFormat unormFormat = isSRGB(format) ? convertToUNORM(format) : format;
            Logger::debug(std::to_string(srgbFormat) + " " + std::to_string(unormFormat));

            VkFormat formats[] = {unormFormat, srgbFormat};

            VkImageFormatListCreateInfoKHR imageFormatListCreateInfo;
            if (pLogicalDevice->supportsMutableFormat)
            {
                modifiedCreateInfo.imageUsage =
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    | VK_IMAGE_USAGE_SAMPLED_BIT; // we want to use the swapchain images as output of the graphics pipeline
                modifiedCreateInfo.flags |= VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;
                // TODO what if the application already uses multiple formats for the swapchain?

                imageFormatListCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
                imageFormatListCreateInfo.pNext           = modifiedCreateInfo.pNext;
                imageFormatListCreateInfo.viewFormatCount = (srgbFormat == unormFormat) ? 1 : 2;
                imageFormatListCreateInfo.pViewFormats    = formats;

                modifiedCreateInfo.pNext = &imageFormatListCreateInfo;
            }

            modifiedCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            Logger::debug("format " + std::to_string(modifiedCreateInfo.imageFormat));

            VkResult result = pDispatch->CreateSwapchainKHR(device, &modifiedCreateInfo, pAllocator, pSwapchain);
            if (result != VK_SUCCESS)
            {
                Logger::trace("vkCreateSwapchainKHR KO");
                return result;
            }

            swapchainMap::create(*pSwapchain,
                                 swapchainMapData{
                                     .pLogicalDevice      = pLogicalDevice.get(),
                                     .swapchainCreateInfo = *pCreateInfo,
                                     .imageExtent         = modifiedCreateInfo.imageExtent,
                                     .format              = modifiedCreateInfo.imageFormat,
                                     .imageCount          = 0,
                                 });

            pConfig = std::make_shared<vkBasalt::Config>();

            return result;
        }

        static auto GetSwapchainImagesKHR(
            const vkroots::VkDeviceDispatch* pDispatch, VkDevice device, VkSwapchainKHR swapchain, uint32_t* pCount, VkImage* pSwapchainImages)
        {
            scoped_lock l(globalLock);
            Logger::trace("vkGetSwapchainImagesKHR " + std::to_string(*pCount));

            auto pLogicalDevice = deviceMap::get(device);

            if (!pLogicalDevice->commandPool)
            {
                uint32_t count;

                pDispatch->pPhysicalDeviceDispatch->pInstanceDispatch->GetPhysicalDeviceQueueFamilyProperties(
                    pLogicalDevice->physicalDevice, &count, nullptr);

                std::vector<VkQueueFamilyProperties> queueProperties(count);

                pDispatch->pPhysicalDeviceDispatch->pInstanceDispatch->GetPhysicalDeviceQueueFamilyProperties(
                    pLogicalDevice->physicalDevice, &count, queueProperties.data());
                for (uint32_t i = 0; i < count; i++)
                {
                    if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                    {
                        pDispatch->GetDeviceQueue(pLogicalDevice->device, i, 0, &pLogicalDevice->queue);

                        VkCommandPoolCreateInfo commandPoolCreateInfo;
                        commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                        commandPoolCreateInfo.pNext            = nullptr;
                        commandPoolCreateInfo.flags            = 0;
                        commandPoolCreateInfo.queueFamilyIndex = i;

                        Logger::debug("Found graphics capable queue");
                        pDispatch->CreateCommandPool(pLogicalDevice->device, &commandPoolCreateInfo, nullptr, &pLogicalDevice->commandPool);
                        pLogicalDevice->queueFamilyIndex = i;

                        break;
                    }
                }

                if (!pLogicalDevice->queue)
                    Logger::err("Did not find a graphics queue!");
            }

            if (pSwapchainImages == nullptr)
            {
                return pDispatch->GetSwapchainImagesKHR(device, swapchain, pCount, pSwapchainImages);
            }

            auto pLogicalSwapchain = swapchainMap::get(swapchain);

            // If the images got already requested once, return them again instead of creating new images
            if (pLogicalSwapchain->fakeImages.size())
            {
                *pCount = std::min<uint32_t>(*pCount, pLogicalSwapchain->imageCount);
                std::memcpy(pSwapchainImages, pLogicalSwapchain->fakeImages.data(), sizeof(VkImage) * (*pCount));
                return *pCount < pLogicalSwapchain->imageCount ? VK_INCOMPLETE : VK_SUCCESS;
            }

            pDispatch->GetSwapchainImagesKHR(device, swapchain, &pLogicalSwapchain->imageCount, nullptr);
            pLogicalSwapchain->images.resize(pLogicalSwapchain->imageCount);
            pDispatch->GetSwapchainImagesKHR(device, swapchain, &pLogicalSwapchain->imageCount, pLogicalSwapchain->images.data());

            auto effectStrings = pConfig->getOption<std::vector<std::string>>("effects", {"cas"});

            // create 1 more set of images when we can't use the swapchain it self
            uint32_t fakeImageCount = pLogicalSwapchain->imageCount * (effectStrings.size() + !pLogicalDevice->supportsMutableFormat);

            pLogicalSwapchain->fakeImages = createFakeSwapchainImages(
                pDispatch, pLogicalDevice.get(), pLogicalSwapchain->swapchainCreateInfo, fakeImageCount, pLogicalSwapchain->fakeImageMemory);
            Logger::debug("created fake swapchain images");

            VkFormat unormFormat = convertToUNORM(pLogicalSwapchain->format);
            VkFormat srgbFormat  = convertToSRGB(pLogicalSwapchain->format);

            for (uint32_t i = 0; i < effectStrings.size(); i++)
            {
                Logger::debug("current effectString " + effectStrings[i]);
                std::vector<VkImage> firstImages(pLogicalSwapchain->fakeImages.begin() + pLogicalSwapchain->imageCount * i,
                                                 pLogicalSwapchain->fakeImages.begin() + pLogicalSwapchain->imageCount * (i + 1));
                Logger::debug(std::to_string(firstImages.size()) + " images in firstImages");
                std::vector<VkImage> secondImages;
                if (i == effectStrings.size() - 1)
                {
                    secondImages = pLogicalDevice->supportsMutableFormat
                                       ? pLogicalSwapchain->images
                                       : std::vector<VkImage>(pLogicalSwapchain->fakeImages.end() - pLogicalSwapchain->imageCount,
                                                              pLogicalSwapchain->fakeImages.end());
                    Logger::debug("using swapchain images as second images");
                }
                else
                {
                    secondImages = std::vector<VkImage>(pLogicalSwapchain->fakeImages.begin() + pLogicalSwapchain->imageCount * (i + 1),
                                                        pLogicalSwapchain->fakeImages.begin() + pLogicalSwapchain->imageCount * (i + 2));
                    Logger::debug("not using swapchain images as second images");
                }
                Logger::debug(std::to_string(secondImages.size()) + " images in secondImages");
                if (effectStrings[i] == std::string("fxaa"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new FxaaEffect(
                        pDispatch, pLogicalDevice.get(), srgbFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created FxaaEffect");
                }
                else if (effectStrings[i] == std::string("cas"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new CasEffect(
                        pDispatch, pLogicalDevice.get(), unormFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created CasEffect");
                }
                else if (effectStrings[i] == std::string("deband"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new DebandEffect(
                        pDispatch, pLogicalDevice.get(), unormFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created DebandEffect");
                }
                else if (effectStrings[i] == std::string("smaa"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new SmaaEffect(
                        pDispatch, pLogicalDevice.get(), unormFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created SmaaEffect");
                }
                else if (effectStrings[i] == std::string("lut"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new LutEffect(
                        pDispatch, pLogicalDevice.get(), unormFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created LutEffect");
                }
                else if (effectStrings[i] == std::string("dls"))
                {
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new DlsEffect(
                        pDispatch, pLogicalDevice.get(), unormFormat, pLogicalSwapchain->imageExtent, firstImages, secondImages, pConfig.get())));
                    Logger::debug("created DlsEffect");
                }
                else
                {
#if !defined(DISABLE_RESHADEFX) || DISABLE_RESHADEFX == 0
                    pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new ReshadeEffect(pDispatch,
                                                                                                   pLogicalDevice.get(),
                                                                                                   pLogicalSwapchain->format,
                                                                                                   pLogicalSwapchain->imageExtent,
                                                                                                   firstImages,
                                                                                                   secondImages,
                                                                                                   pConfig.get(),
                                                                                                   effectStrings[i])));
                    Logger::debug("created ReshadeEffect");
#else
                    Logger::debug("ReshadeEffects disabled");
#endif
                }
            }

            if (!pLogicalDevice->supportsMutableFormat)
            {
                pLogicalSwapchain->effects.push_back(std::shared_ptr<Effect>(new TransferEffect(
                    pDispatch,
                    pLogicalDevice.get(),
                    pLogicalSwapchain->format,
                    pLogicalSwapchain->imageExtent,
                    std::vector<VkImage>(pLogicalSwapchain->fakeImages.end() - pLogicalSwapchain->imageCount, pLogicalSwapchain->fakeImages.end()),
                    pLogicalSwapchain->images,
                    pConfig.get())));
            }

            VkImageView depthImageView = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthImageViews[0] : VK_NULL_HANDLE;
            VkImage     depthImage     = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthImages[0] : VK_NULL_HANDLE;
            VkFormat    depthFormat    = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthFormats[0] : VK_FORMAT_UNDEFINED;

            Logger::debug("effect string count: " + std::to_string(effectStrings.size()));
            Logger::debug("effect count: " + std::to_string(pLogicalSwapchain->effects.size()));

            pLogicalSwapchain->commandBuffersEffect = allocateCommandBuffer(pDispatch, pLogicalDevice.get(), pLogicalSwapchain->imageCount);
            Logger::debug("allocated ComandBuffers " + std::to_string(pLogicalSwapchain->commandBuffersEffect.size()) + " for swapchain "
                          + convertToString(swapchain));

            writeCommandBuffers(pDispatch,
                                pLogicalDevice.get(),
                                pLogicalSwapchain->effects,
                                depthImage,
                                depthImageView,
                                depthFormat,
                                pLogicalSwapchain->commandBuffersEffect);
            Logger::debug("wrote CommandBuffers");

            pLogicalSwapchain->semaphores = createSemaphores(pDispatch, pLogicalDevice.get(), pLogicalSwapchain->imageCount);
            Logger::debug("created semaphores");
            for (unsigned int i = 0; i < pLogicalSwapchain->imageCount; i++)
            {
                Logger::debug(std::to_string(i) + " written commandbuffer " + convertToString(pLogicalSwapchain->commandBuffersEffect[i]));
            }
            Logger::trace("vkGetSwapchainImagesKHR");

            pLogicalSwapchain->defaultTransfer = std::shared_ptr<Effect>(new TransferEffect(
                pDispatch,
                pLogicalDevice.get(),
                pLogicalSwapchain->format,
                pLogicalSwapchain->imageExtent,
                std::vector<VkImage>(pLogicalSwapchain->fakeImages.begin(), pLogicalSwapchain->fakeImages.begin() + pLogicalSwapchain->imageCount),
                pLogicalSwapchain->images,
                pConfig.get()));

            pLogicalSwapchain->commandBuffersNoEffect = allocateCommandBuffer(pDispatch, pLogicalDevice.get(), pLogicalSwapchain->imageCount);

            writeCommandBuffers(pDispatch,
                                pLogicalDevice.get(),
                                {pLogicalSwapchain->defaultTransfer},
                                VK_NULL_HANDLE,
                                VK_NULL_HANDLE,
                                VK_FORMAT_UNDEFINED,
                                pLogicalSwapchain->commandBuffersNoEffect);

            for (unsigned int i = 0; i < pLogicalSwapchain->imageCount; i++)
            {
                Logger::debug(std::to_string(i) + " written commandbuffer " + convertToString(pLogicalSwapchain->commandBuffersNoEffect[i]));
            }

            *pCount = std::min<uint32_t>(*pCount, pLogicalSwapchain->imageCount);
            std::memcpy(pSwapchainImages, pLogicalSwapchain->fakeImages.data(), sizeof(VkImage) * (*pCount));
            return *pCount < pLogicalSwapchain->imageCount ? VK_INCOMPLETE : VK_SUCCESS;
        }

        static auto QueuePresentKHR(const vkroots::VkDeviceDispatch* pDispatch, VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
        {
            scoped_lock l(globalLock);

            static uint32_t keySymbol = convertToKeySym(pConfig->getOption<std::string>("toggleKey", "Home"));

            static bool pressed       = false;
            static bool presentEffect = pConfig->getOption<bool>("enableOnLaunch", true);

            if (isKeyPressed(keySymbol))
            {
                if (!pressed)
                {
                    presentEffect = !presentEffect;
                    pressed       = true;
                }
            }
            else
            {
                pressed = false;
            }

            std::vector<VkSemaphore> presentSemaphores;
            presentSemaphores.reserve(pPresentInfo->swapchainCount);

            std::vector<VkPipelineStageFlags> waitStages(pPresentInfo->waitSemaphoreCount, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            for (unsigned int i = 0; i < (*pPresentInfo).swapchainCount; i++)
            {
                uint32_t       index             = (*pPresentInfo).pImageIndices[i];
                VkSwapchainKHR swapchain         = (*pPresentInfo).pSwapchains[i];
                auto           pLogicalSwapchain = swapchainMap::get(swapchain);

                for (auto& effect : pLogicalSwapchain->effects)
                {
                    effect->updateEffect(pDispatch);
                }

                VkSubmitInfo submitInfo;
                submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pNext              = nullptr;
                submitInfo.waitSemaphoreCount = i == 0 ? pPresentInfo->waitSemaphoreCount : 0;
                submitInfo.pWaitSemaphores    = i == 0 ? pPresentInfo->pWaitSemaphores : nullptr;
                submitInfo.pWaitDstStageMask  = i == 0 ? waitStages.data() : nullptr;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers =
                    presentEffect ? &(pLogicalSwapchain->commandBuffersEffect[index]) : &(pLogicalSwapchain->commandBuffersNoEffect[index]);
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores    = &(pLogicalSwapchain->semaphores[index]);

                presentSemaphores.push_back(pLogicalSwapchain->semaphores[index]);

                VkResult vr = pDispatch->QueueSubmit(pLogicalSwapchain->pLogicalDevice->queue, 1, &submitInfo, VK_NULL_HANDLE);

                if (vr != VK_SUCCESS)
                {
                    return vr;
                }
            }

            VkPresentInfoKHR presentInfo   = *pPresentInfo;
            presentInfo.waitSemaphoreCount = presentSemaphores.size();
            presentInfo.pWaitSemaphores    = presentSemaphores.data();

            return pDispatch->QueuePresentKHR(queue, &presentInfo);
        }

        static auto DestroySwapchainKHR(const vkroots::VkDeviceDispatch* pDispatch,
                                        VkDevice                         device,
                                        VkSwapchainKHR                   swapchain,
                                        const VkAllocationCallbacks*     pAllocator)
        {
            if (!swapchain)
                return;

            scoped_lock l(globalLock);

            // we need to delete the infos of the oldswapchain
            Logger::trace("vkDestroySwapchainKHR " + convertToString(swapchain));
            swapchainMap::get(swapchain)->destroy(pDispatch);
            swapchainMap::remove(swapchain);

            pDispatch->DestroySwapchainKHR(device, swapchain, pAllocator);
        }

#if !defined(DISABLE_DEPTH_CAPTURE) || DISABLE_DEPTH_CAPTURE == 0
        static auto CreateImage(const vkroots::VkDeviceDispatch* pDispatch,
                                VkDevice                         device,
                                const VkImageCreateInfo*         pCreateInfo,
                                const VkAllocationCallbacks*     pAllocator,
                                VkImage*                         pImage)
        {
            scoped_lock l(globalLock);

            auto pLogicalDevice = deviceMap::get(device);
            if (isDepthFormat(pCreateInfo->format) && pCreateInfo->samples == VK_SAMPLE_COUNT_1_BIT
                && ((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
            {
                Logger::debug("detected depth image with format: " + convertToString(pCreateInfo->format));
                Logger::debug(std::to_string(pCreateInfo->extent.width) + "x" + std::to_string(pCreateInfo->extent.height));
                Logger::debug(std::to_string((pCreateInfo->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                             == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));

                VkImageCreateInfo modifiedCreateInfo = *pCreateInfo;
                modifiedCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
                VkResult result = pDispatch->CreateImage(device, &modifiedCreateInfo, pAllocator, pImage);
                pLogicalDevice->depthImages.push_back(*pImage);
                pLogicalDevice->depthFormats.push_back(pCreateInfo->format);

                return result;
            }
            else
            {
                return pDispatch->CreateImage(device, pCreateInfo, pAllocator, pImage);
            }
        }

        static auto
        BindImageMemory(const vkroots::VkDeviceDispatch* pDispatch, VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
        {
            scoped_lock l(globalLock);

            auto pLogicalDevice = deviceMap::get(device);

            VkResult result = pDispatch->BindImageMemory(device, image, memory, memoryOffset);
            // TODO what if the application creates more than one image before binding memory?
            if (pLogicalDevice->depthImages.size() && image == pLogicalDevice->depthImages.back())
            {
                Logger::debug("before creating depth image view");
                VkImageView depthImageView = createImageViews(pDispatch,
                                                              pLogicalDevice.get(),
                                                              pLogicalDevice->depthFormats[pLogicalDevice->depthImages.size() - 1],
                                                              {image},
                                                              VK_IMAGE_VIEW_TYPE_2D,
                                                              VK_IMAGE_ASPECT_DEPTH_BIT)[0];

                VkFormat depthFormat = pLogicalDevice->depthFormats[pLogicalDevice->depthImages.size() - 1];

                Logger::debug("created depth image view");
                pLogicalDevice->depthImageViews.push_back(depthImageView);
                if (pLogicalDevice->depthImageViews.size() > 1)
                {
                    return result;
                }

                for (auto& it : swapchainMap::raw())
                {
                    LogicalSwapchain* pLogicalSwapchain = it.second.get();
                    if (pLogicalSwapchain->pLogicalDevice == pLogicalDevice.get())
                    {
                        if (pLogicalSwapchain->commandBuffersEffect.size())
                        {
                            pDispatch->FreeCommandBuffers(pLogicalDevice->device,
                                                          pLogicalDevice->commandPool,
                                                          pLogicalSwapchain->commandBuffersEffect.size(),
                                                          pLogicalSwapchain->commandBuffersEffect.data());
                            pLogicalSwapchain->commandBuffersEffect.clear();
                            pLogicalSwapchain->commandBuffersEffect =
                                allocateCommandBuffer(pDispatch, pLogicalDevice.get(), pLogicalSwapchain->imageCount);
                            Logger::debug("allocated CommandBuffers for swapchain " + convertToString(it.first));

                            writeCommandBuffers(pDispatch,
                                                pLogicalDevice.get(),
                                                pLogicalSwapchain->effects,
                                                image,
                                                depthImageView,
                                                depthFormat,
                                                pLogicalSwapchain->commandBuffersEffect);
                            Logger::debug("wrote CommandBuffers");
                        }
                    }
                }
            }
            return result;
        }

        static auto DestroyImage(const vkroots::VkDeviceDispatch* pDispatch, VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
        {
            if (!image)
                return;

            scoped_lock l(globalLock);

            auto pLogicalDevice = deviceMap::get(device);

            for (uint32_t i = 0; i < pLogicalDevice->depthImages.size(); i++)
            {
                if (pLogicalDevice->depthImages[i] == image)
                {
                    pLogicalDevice->depthImages.erase(pLogicalDevice->depthImages.begin() + i);
                    // TODO what if a image gets destroyed before binding memory?
                    if (pLogicalDevice->depthImageViews.size() - 1 >= i)
                    {
                        pDispatch->DestroyImageView(pLogicalDevice->device, pLogicalDevice->depthImageViews[i], nullptr);
                        pLogicalDevice->depthImageViews.erase(pLogicalDevice->depthImageViews.begin() + i);
                    }
                    pLogicalDevice->depthFormats.erase(pLogicalDevice->depthFormats.begin() + i);

                    VkImageView depthImageView = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthImageViews[0] : VK_NULL_HANDLE;
                    VkImage     depthImage     = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthImages[0] : VK_NULL_HANDLE;
                    VkFormat    depthFormat    = pLogicalDevice->depthImageViews.size() ? pLogicalDevice->depthFormats[0] : VK_FORMAT_UNDEFINED;
                    for (auto& it : swapchainMap::raw())
                    {
                        LogicalSwapchain* pLogicalSwapchain = it.second.get();
                        if (pLogicalSwapchain->pLogicalDevice == pLogicalDevice.get())
                        {
                            if (pLogicalSwapchain->commandBuffersEffect.size())
                            {
                                pDispatch->FreeCommandBuffers(pLogicalDevice->device,
                                                              pLogicalDevice->commandPool,
                                                              pLogicalSwapchain->commandBuffersEffect.size(),
                                                              pLogicalSwapchain->commandBuffersEffect.data());
                                pLogicalSwapchain->commandBuffersEffect.clear();
                                pLogicalSwapchain->commandBuffersEffect =
                                    allocateCommandBuffer(pDispatch, pLogicalDevice.get(), pLogicalSwapchain->imageCount);
                                Logger::debug("allocated CommandBuffers for swapchain " + convertToString(it.first));

                                writeCommandBuffers(pDispatch,
                                                    pLogicalDevice.get(),
                                                    pLogicalSwapchain->effects,
                                                    depthImage,
                                                    depthImageView,
                                                    depthFormat,
                                                    pLogicalSwapchain->commandBuffersEffect);
                                Logger::debug("wrote CommandBuffers");
                            }
                        }
                    }
                }
            }
            pDispatch->DestroyImage(pLogicalDevice->device, image, pAllocator);
        }
#endif

    }; // VkDeviceOverrides
} // namespace vkBasalt

// Export
VKROOTS_DEFINE_LAYER_INTERFACES(vkBasalt::VkInstanceOverrides, vkroots::NoOverrides, vkBasalt::VkDeviceOverrides);
VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE_EXT(vkBasalt::deviceMap);
VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE_EXT(vkBasalt::swapchainMap);
