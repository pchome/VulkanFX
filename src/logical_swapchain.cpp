#include "logical_swapchain.hpp"
#include <format>

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

            for (uint32_t i = 0; i < fakeImages.size(); i++)
            {
                Logger::debug(std::format("deleting fake image {}", i));
                vmaDestroyImage(pLogicalDevice->allocator, fakeImages[i], fakeImageMemoryBlocks[i]);
            }
            Logger::debug("after free fakeImages");

            for (auto& semaphore : semaphores)
            {
                pDispatch->DestroySemaphore(pLogicalDevice->device, semaphore, nullptr);
            }
            Logger::debug("after DestroySemaphore");

            fakeImages.clear();
            fakeImageMemoryBlocks.clear();
        }
    }
} // namespace VulkanFX
