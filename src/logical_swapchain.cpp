#include "logical_swapchain.hpp"

namespace vkBasalt
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

            pDispatch->FreeMemory(pLogicalDevice->device, fakeImageMemory, nullptr);

            for (uint32_t i = 0; i < fakeImages.size(); i++)
            {
                pDispatch->DestroyImage(pLogicalDevice->device, fakeImages[i], nullptr);
            }

            for (unsigned int i = 0; i < imageCount; i++)
            {
                pDispatch->DestroySemaphore(pLogicalDevice->device, semaphores[i], nullptr);
            }
            Logger::debug("after DestroySemaphore");
        }
    }
} // namespace vkBasalt
