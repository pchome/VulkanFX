#include "memory.hpp"

namespace VulkanFX
{
    auto findMemoryTypeIndex(const vkroots::VkDeviceDispatch* pDispatch,
                             LogicalDevice*                   pLogicalDevice,
                             uint32_t                         typeFilter,
                             VkMemoryPropertyFlags            properties) -> uint32_t
    {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        pDispatch->pPhysicalDeviceDispatch->pInstanceDispatch->GetPhysicalDeviceMemoryProperties(pLogicalDevice->physicalDevice,
                                                                                                 &physicalDeviceMemoryProperties);
        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        Logger::err("Found no correct memory type");
        return 0x70AD;
    }
} // namespace VulkanFX
