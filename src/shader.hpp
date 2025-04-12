#pragma once
#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED
#include <vector>
#include <cstdlib>
#include "vulkan_include.hpp"

namespace vkBasalt
{
    void createShaderModule(const vkroots::VkDeviceDispatch* pDispatch,
                            LogicalDevice*                   pLogicalDevice,
                            const std::vector<char>&         code,
                            VkShaderModule*                  shaderModule);
    void createShaderModule(const vkroots::VkDeviceDispatch* pDispatch,
                            LogicalDevice*                   pLogicalDevice,
                            const std::vector<uint32_t>&     code,
                            VkShaderModule*                  shaderModule);
} // namespace vkBasalt

#endif // SHADER_HPP_INCLUDED
