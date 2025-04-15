#pragma once
#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED
#include <iostream>
#include <mutex>
#include <string>
#include <memory>
#include <functional>

namespace VulkanFX
{

    enum class LogLevel : uint32_t
    {
        Trace = 0,
        Debug = 1,
        Info  = 2,
        Warn  = 3,
        Error = 4,
        None  = 5,
    };

    class Logger
    {

    public:
        Logger();
        ~Logger();

        static void trace(const std::string& message);
        static void debug(const std::string& message);
        static void info(const std::string& message);
        static void warn(const std::string& message);
        static void err(const std::string& message);
        static void log(LogLevel level, const std::string& message);

        static auto logLevel() -> LogLevel
        {
            return s_instance.m_minLevel;
        }

    private:
        static Logger s_instance;

        const LogLevel m_minLevel;

        std::mutex m_mutex;

        std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> m_outStream;

        void emitMsg(LogLevel level, const std::string& message);

        static auto getMinLogLevel() -> LogLevel;

        static auto getFileName() -> std::string;
    };

} // namespace VulkanFX

#endif // LOGGER_HPP_INCLUDED
