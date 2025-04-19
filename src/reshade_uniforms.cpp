#include "reshade_uniforms.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cmath>

#include <algorithm>

#include "logger.hpp"

namespace VulkanFX
{
    void enumerateReShadeUniforms(reshadefx::effect_module module)
    {
        for (auto& uniform : module.uniforms)
        {
            auto src_annotation = std::ranges::find_if(uniform.annotations, [](const auto& a) { return a.name == "source"; });
            std::string source = src_annotation != uniform.annotations.end() ? src_annotation->value.string_data : "(empty)";
            Logger::debug("uniform: '" + source + "'"); // TODO: can be empty or unknown
            Logger::debug("name:     " + uniform.name);
            Logger::debug("size:     " + std::to_string(uniform.size));
            Logger::debug("offset:   " + std::to_string(uniform.offset));
        }
    }

    auto createReShadeUniforms(reshadefx::effect_module module) -> std::vector<std::shared_ptr<ReShadeUniform>>
    {
        std::vector<std::shared_ptr<ReShadeUniform>> uniforms;
        for (auto& uniform : module.uniforms)
        {
            auto src_annotation = std::ranges::find_if(uniform.annotations, [](const auto& a) { return a.name == "source"; });
            if (src_annotation == uniform.annotations.end())
            {
                continue;
            }
            std::string source = src_annotation->value.string_data;
            if (source == "frametime")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new FrameTimeUniform(uniform)));
            }
            else if (source == "framecount")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new FrameCountUniform(uniform)));
            }
            else if (source == "date")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new DateUniform(uniform)));
            }
            else if (source == "timer")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new TimerUniform(uniform)));
            }
            else if (source == "pingpong")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new PingPongUniform(uniform)));
            }
            else if (source == "random")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new RandomUniform(uniform)));
            }
            else if (source == "key")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new KeyUniform(uniform)));
            }
            else if (source == "mousebutton")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new MouseButtonUniform(uniform)));
            }
            else if (source == "mousepoint")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new MousePointUniform(uniform)));
            }
            else if (source == "mousedelta")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new MouseDeltaUniform(uniform)));
            }
            else if (source == "bufready_depth")
            {
                uniforms.push_back(std::shared_ptr<ReShadeUniform>(new DepthUniform(uniform)));
            }
        }
        return uniforms;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    FrameTimeUniform::FrameTimeUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "frametime")
        {
            Logger::err("Tried to create a FrameTimeUniform from a non frametime uniform_info");
        }
        lastFrame = std::chrono::steady_clock::now();
        offset    = uniformInfo.offset;
        size      = uniformInfo.size;
    }
    void FrameTimeUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        auto                                     currentFrame = std::chrono::steady_clock::now();
        std::chrono::duration<float, std::milli> duration     = currentFrame - lastFrame;
        lastFrame                                             = currentFrame;
        float frametime                                       = duration.count();
        vmaCopyMemoryToAllocation(allocator, &(frametime), stagingBufferMemory, offset, sizeof(float));
    }
    FrameTimeUniform::~FrameTimeUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    FrameCountUniform::FrameCountUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "framecount")
        {
            Logger::err("Tried to create a FrameCountUniform from a non framecount uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void FrameCountUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        vmaCopyMemoryToAllocation(allocator, &(count), stagingBufferMemory, offset, sizeof(int32_t));
        count++;
    }
    FrameCountUniform::~FrameCountUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    DateUniform::DateUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "date")
        {
            Logger::err("Tried to create a DateUniform from a non date uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void DateUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        auto        now         = std::chrono::system_clock::now();
        std::time_t nowC        = std::chrono::system_clock::to_time_t(now);
        struct tm*  currentTime = std::localtime(&nowC);
        float       year        = 1900.0f + static_cast<float>(currentTime->tm_year);
        float       month       = 1.0f + static_cast<float>(currentTime->tm_mon);
        float       day         = static_cast<float>(currentTime->tm_mday);
        float       seconds     = static_cast<float>((currentTime->tm_hour * 60 + currentTime->tm_min) * 60 + currentTime->tm_sec);
        float       date[]      = {year, month, day, seconds};
        vmaCopyMemoryToAllocation(allocator, date, stagingBufferMemory, offset, sizeof(float) * 4);
    }
    DateUniform::~DateUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    TimerUniform::TimerUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "timer")
        {
            Logger::err("Tried to create a TimerUniform from a non timer uniform_info");
        }
        start  = std::chrono::steady_clock::now();
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void TimerUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        auto                                     currentFrame = std::chrono::steady_clock::now();
        std::chrono::duration<float, std::milli> duration     = currentFrame - start;
        float                                    timer        = duration.count();
        vmaCopyMemoryToAllocation(allocator, &(timer), stagingBufferMemory, offset, sizeof(float));
    }
    TimerUniform::~TimerUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    PingPongUniform::PingPongUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "pingpong")
        {
            Logger::err("Tried to create a PingPongUniform from a non pingpong uniform_info");
        }
        if (auto minAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "min"; });
            minAnnotation != uniformInfo.annotations.end())
        {
            min = minAnnotation->type.is_floating_point() ? minAnnotation->value.as_float[0] : static_cast<float>(minAnnotation->value.as_int[0]);
        }
        if (auto maxAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "max"; });
            maxAnnotation != uniformInfo.annotations.end())
        {
            max = maxAnnotation->type.is_floating_point() ? maxAnnotation->value.as_float[0] : static_cast<float>(maxAnnotation->value.as_int[0]);
        }
        if (auto smoothingAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "smoothing"; });
            smoothingAnnotation != uniformInfo.annotations.end())
        {
            smoothing = smoothingAnnotation->type.is_floating_point() ? smoothingAnnotation->value.as_float[0]
                                                                      : static_cast<float>(smoothingAnnotation->value.as_int[0]);
        }
        if (auto stepAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "step"; });
            stepAnnotation != uniformInfo.annotations.end())
        {
            stepMin =
                stepAnnotation->type.is_floating_point() ? stepAnnotation->value.as_float[0] : static_cast<float>(stepAnnotation->value.as_int[0]);
            stepMax =
                stepAnnotation->type.is_floating_point() ? stepAnnotation->value.as_float[1] : static_cast<float>(stepAnnotation->value.as_int[1]);
        }

        lastFrame = std::chrono::steady_clock::now();

        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void PingPongUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        auto currentFrame = std::chrono::steady_clock::now();

        std::chrono::duration<float, std::ratio<1>> frameTime = currentFrame - lastFrame;

        float increment = stepMax == 0 ? stepMin : (stepMin + std::fmod(static_cast<float>(std::rand()), stepMax - stepMin + 1.0f));
        if (currentValue[1] >= 0)
        {
            increment = std::max(increment - std::max(0.0f, smoothing - (max - currentValue[0])), 0.05f);
            increment *= frameTime.count();

            if ((currentValue[0] += increment) >= max)
            {
                currentValue[0] = max, currentValue[1] = -1.0f;
            }
        }
        else
        {
            increment = std::max(increment - std::max(0.0f, smoothing - (currentValue[0] - min)), 0.05f);
            increment *= frameTime.count();

            if ((currentValue[0] -= increment) <= min)
            {
                currentValue[0] = min, currentValue[1] = 1.0f;
            }
        }
        vmaCopyMemoryToAllocation(allocator, currentValue, stagingBufferMemory, offset, sizeof(float) * 2);
    }
    PingPongUniform::~PingPongUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    RandomUniform::RandomUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "random")
        {
            Logger::err("Tried to create a RandomUniform from a non random uniform_info");
        }
        if (auto minAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "min"; });
            minAnnotation != uniformInfo.annotations.end())
        {
            min = minAnnotation->type.is_integral() ? minAnnotation->value.as_int[0] : static_cast<int>(minAnnotation->value.as_float[0]);
        }
        if (auto maxAnnotation =
                std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "max"; });
            maxAnnotation != uniformInfo.annotations.end())
        {
            max = maxAnnotation->type.is_integral() ? maxAnnotation->value.as_int[0] : static_cast<int>(maxAnnotation->value.as_float[0]);
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void RandomUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        int32_t value = min + (std::rand() % (max - min + 1));
        vmaCopyMemoryToAllocation(allocator, &(value), stagingBufferMemory, offset, sizeof(int32_t));
    }
    RandomUniform::~RandomUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    KeyUniform::KeyUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "key")
        {
            Logger::err("Tried to create a KeyUniform from a non key uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void KeyUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        VkBool32 keyDown = VK_FALSE; // TODO
        vmaCopyMemoryToAllocation(allocator, &(keyDown), stagingBufferMemory, offset, sizeof(VkBool32));
    }
    KeyUniform::~KeyUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    MouseButtonUniform::MouseButtonUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "mousebutton")
        {
            Logger::err("Tried to create a MouseButtonUniform from a non mousebutton uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void MouseButtonUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        VkBool32 keyDown = VK_FALSE; // TODO
        vmaCopyMemoryToAllocation(allocator, &(keyDown), stagingBufferMemory, offset, sizeof(VkBool32));
    }
    MouseButtonUniform::~MouseButtonUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    MousePointUniform::MousePointUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "mousepoint")
        {
            Logger::err("Tried to create a MousePointUniform from a non mousepoint uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void MousePointUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        float point[2] = {0.0f, 0.0f}; // TODO
        vmaCopyMemoryToAllocation(allocator, point, stagingBufferMemory, offset, sizeof(float) * 2);
    }
    MousePointUniform::~MousePointUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    MouseDeltaUniform::MouseDeltaUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "mousedelta")
        {
            Logger::err("Tried to create a MouseDeltaUniform from a non mousedelta uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void MouseDeltaUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        float delta[2] = {0.0f, 0.0f}; // TODO
        vmaCopyMemoryToAllocation(allocator, delta, stagingBufferMemory, offset, sizeof(float) * 2);
    }
    MouseDeltaUniform::~MouseDeltaUniform()
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    DepthUniform::DepthUniform(reshadefx::uniform uniformInfo)
    {
        auto source = std::ranges::find_if(uniformInfo.annotations, [](const auto& a) { return a.name == "source"; });
        if (source->value.string_data != "bufready_depth")
        {
            Logger::err("Tried to create a DepthUniform from a non bufready_depth uniform_info");
        }
        offset = uniformInfo.offset;
        size   = uniformInfo.size;
    }
    void DepthUniform::update(VmaAllocator allocator, VmaAllocation stagingBufferMemory)
    {
        VkBool32 hasDepth = VK_FALSE; // TODO
        vmaCopyMemoryToAllocation(allocator, &(hasDepth), stagingBufferMemory, offset, sizeof(VkBool32));
    }
    DepthUniform::~DepthUniform()
    {
    }
} // namespace VulkanFX
