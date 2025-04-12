#include "buffer.hpp"
#include "memory.hpp"

namespace vkBasalt
{
    void createBuffer(const vkroots::VkDeviceDispatch* pDispatch,
                      LogicalDevice*                   pLogicalDevice,
                      VkDeviceSize                     size,
                      VkBufferUsageFlags               usage,
                      VkMemoryPropertyFlags            properties,
                      VkBuffer&                        buffer,
                      VkDeviceMemory&                  bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = {};

        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = pDispatch->CreateBuffer(pLogicalDevice->device, &bufferInfo, nullptr, &buffer);
        ASSERT_VULKAN(result);

        VkMemoryRequirements memRequirements;
        pDispatch->GetBufferMemoryRequirements(pLogicalDevice->device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};

        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryTypeIndex(pDispatch, pLogicalDevice, memRequirements.memoryTypeBits, properties);

        result = pDispatch->AllocateMemory(pLogicalDevice->device, &allocInfo, nullptr, &bufferMemory);
        ASSERT_VULKAN(result);

        result = pDispatch->BindBufferMemory(pLogicalDevice->device, buffer, bufferMemory, 0);
        ASSERT_VULKAN(result);
    }

} // namespace vkBasalt
