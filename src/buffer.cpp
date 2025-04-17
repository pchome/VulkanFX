#include "buffer.hpp"

namespace VulkanFX
{
    void createBuffer(const vkroots::VkDeviceDispatch* pDispatch,
                      LogicalDevice*                   pLogicalDevice,
                      VkDeviceSize                     size,
                      VkBufferUsageFlags               usage,
                      VkMemoryPropertyFlags            properties,
                      VkBuffer&                        buffer,
                      VmaAllocation&                   bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = {};

        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocInfo.preferredFlags = properties;


        VkResult result = vmaCreateBuffer(pLogicalDevice->allocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr);
        ASSERT_VULKAN(result);
    }

} // namespace VulkanFX
