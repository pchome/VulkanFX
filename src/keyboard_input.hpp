#pragma once

#include <cstdint>
#include <string>

namespace VulkanFX
{
    auto convertToKeySym(std::string key) -> uint32_t;
    auto isKeyPressed(uint32_t ks) -> bool;
} // namespace VulkanFX
