#pragma once
#ifndef RESHADE_UNIFORMS_HPP_INCLUDED
#define RESHADE_UNIFORMS_HPP_INCLUDED
#include <vector>
#include <chrono>
#include <memory>

#include "vulkan_include.hpp"

#include "reshade/effect_module.hpp"

namespace VulkanFX
{
    void enumerateReShadeUniforms(reshadefx::effect_module module);

    class ReShadeUniform
    {
    public:
        void virtual update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) = 0;
        virtual ~ReShadeUniform()                                                      = default;

    protected:
        uint32_t offset;
        uint32_t size;
    };

    auto createReShadeUniforms(reshadefx::effect_module module) -> std::vector<std::shared_ptr<ReShadeUniform>>;

    class FrameTimeUniform : public ReShadeUniform
    {
    public:
        FrameTimeUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~FrameTimeUniform() override;

    private:
        std::chrono::time_point<std::chrono::steady_clock> lastFrame;
    };

    class FrameCountUniform : public ReShadeUniform
    {
    public:
        FrameCountUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~FrameCountUniform() override;

    private:
        int32_t count = 0;
    };

    class DateUniform : public ReShadeUniform
    {
    public:
        DateUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~DateUniform() override;

    private:
        static constexpr int start_year = 1900;
        static constexpr int min_in_hr  = 60;

        std::array<float, 4> date = {start_year, 1, 1, 0};
    };

    class TimerUniform : public ReShadeUniform
    {
    public:
        TimerUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~TimerUniform() override;

    private:
        std::chrono::time_point<std::chrono::steady_clock> start;
    };

    class PingPongUniform : public ReShadeUniform
    {
    public:
        PingPongUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~PingPongUniform() override;

    private:
        static constexpr float step = 0.05f;

        std::chrono::time_point<std::chrono::steady_clock> lastFrame;

        float min       = 0.0f;
        float max       = 0.0f;
        float stepMin   = 0.0f;
        float stepMax   = 0.0f;
        float smoothing = 0.0f;

        std::array<float, 2> currentValue = {0.0f, 1.0f};
    };

    class RandomUniform : public ReShadeUniform
    {
    public:
        RandomUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~RandomUniform() override;

    private:
        int max = 0;
        int min = 0;
    };

    class KeyUniform : public ReShadeUniform
    {
    public:
        KeyUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~KeyUniform() override;
    };

    class MouseButtonUniform : public ReShadeUniform
    {
    public:
        MouseButtonUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~MouseButtonUniform() override;
    };

    class MousePointUniform : public ReShadeUniform
    {
    public:
        MousePointUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~MousePointUniform() override;
    };

    class MouseDeltaUniform : public ReShadeUniform
    {
    public:
        MouseDeltaUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~MouseDeltaUniform() override;
    };

    class DepthUniform : public ReShadeUniform
    {
    public:
        DepthUniform(reshadefx::uniform uniformInfo);
        void update(VmaAllocator allocator, VmaAllocation stagingBufferMemory) override;
        ~DepthUniform() override;
    };
} // namespace VulkanFX

#endif // RESHADE_UNIFORMS_HPP_INCLUDED
