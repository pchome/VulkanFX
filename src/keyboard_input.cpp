#include "keyboard_input.hpp"

// TODO build without X11
#ifndef VULKANFX_X11
#define VULKANFX_X11 1
#endif

#if VULKANFX_X11
#include "keyboard_input_x11.hpp"
#endif

namespace VulkanFX
{
    auto convertToKeySym(std::string key) -> uint32_t
    {
#if VULKANFX_X11
        return convertToKeySymX11(key);
#endif
        return 0u;
    }

    auto isKeyPressed(uint32_t ks) -> bool
    {
#if VULKANFX_X11
        return isKeyPressedX11(ks);
#endif
        return false;
    }
} // namespace VulkanFX
