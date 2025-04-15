#pragma once

#include <cstdint>
#include <string>

namespace VulkanFX
{
    auto convertToKeySymX11(std::string key) -> uint32_t;
    auto isKeyPressedX11(uint32_t ks) -> bool;
} // namespace VulkanFX
