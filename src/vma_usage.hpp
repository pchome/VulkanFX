#pragma once
#include <string>

const auto debug_lvl = std::getenv("VULKANFX_LOG_LEVEL");
const auto vma_debug = std::string(debug_lvl ?: "") == "debug";

#define VMA_VULKAN_VERSION 1002000 // Vulkan 1.2
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#ifndef VMA_DEBUG_LOG_FORMAT
#define VMA_DEBUG_LOG_FORMAT(format, ...) if (vma_debug) do { \
    printf("VulkanFX    vma: "); \
    printf((format), __VA_ARGS__); \
    printf("\n"); \
} while(false)
#endif

#include "vk_mem_alloc.h"
