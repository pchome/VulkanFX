#include <cstring>
#include "effect_cas.hpp"

#include "shader_sources.hpp"

namespace VulkanFX
{
    CasEffect::CasEffect(const vkroots::VkDeviceDispatch* pDispatch,
                         LogicalDevice*                   pLogicalDevice,
                         VkFormat                         format,
                         VkExtent2D                       imageExtent,
                         std::vector<VkImage>             inputImages,
                         std::vector<VkImage>             outputImages,
                         Config*                          pConfig)
    {

        float sharpness = pConfig->getOption<float>("casSharpness", 0.4f);

        vertexCode   = full_screen_triangle_vert;
        fragmentCode = cas_frag;

        VkSpecializationMapEntry sharpnessMapEntry;
        sharpnessMapEntry.constantID = 0;
        sharpnessMapEntry.offset     = 0;
        sharpnessMapEntry.size       = sizeof(float);

        VkSpecializationInfo fragmentSpecializationInfo;
        fragmentSpecializationInfo.mapEntryCount = 1;
        fragmentSpecializationInfo.pMapEntries   = &sharpnessMapEntry;
        fragmentSpecializationInfo.dataSize      = sizeof(float);
        fragmentSpecializationInfo.pData         = &sharpness;

        pVertexSpecInfo   = nullptr;
        pFragmentSpecInfo = &fragmentSpecializationInfo;

        init(pDispatch, pLogicalDevice, format, imageExtent, inputImages, outputImages, pConfig);
    }
    CasEffect::~CasEffect()
    {
    }
} // namespace VulkanFX
