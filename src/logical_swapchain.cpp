#include "logical_swapchain.hpp"

namespace VulkanFX
{
    void LogicalSwapchain::destroy(const vkroots::VkDeviceDispatch* pDispatch)
    {
        if (imageCount > 0)
        {
            effects.clear();
            defaultTransfer.reset();

            pDispatch->FreeCommandBuffers(
                pLogicalDevice->device, pLogicalDevice->commandPool, commandBuffersEffect.size(), commandBuffersEffect.data());
            pDispatch->FreeCommandBuffers(
                pLogicalDevice->device, pLogicalDevice->commandPool, commandBuffersNoEffect.size(), commandBuffersNoEffect.data());
            Logger::debug("after free commandbuffer");

            for (auto& fakeImage : fakeImages)
            {
                vmaDestroyImage(pLogicalDevice->allocator, fakeImage, nullptr);
            }
            Logger::debug("after free fakeImages");

            for (auto& semaphore : semaphores)
            {
                pDispatch->DestroySemaphore(pLogicalDevice->device, semaphore, nullptr);
            }
            Logger::debug("after DestroySemaphore");

            vmaFreeMemory(pLogicalDevice->allocator, fakeImageMemory);
            Logger::debug("after free fakeImagesMemory");
        }
    }
} // namespace VulkanFX
