#pragma once
#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdlib>

namespace vkBasalt
{
    class Config
    {
    public:
        Config();
        Config(const Config& other);

        template<typename T>
        auto getOption(const std::string& option, const T& defaultValue = {}) -> T
        {
            T result = defaultValue;
            parseOption(option, result);
            return result;
        }

    private:
        std::unordered_map<std::string, std::string> options;

        void readConfigLine(std::string line);
        void readConfigFile(std::ifstream& stream);
        void readConfigFromEnv(std::string configLine);

        void parseOption(const std::string& option, int32_t& result);
        void parseOption(const std::string& option, float& result);
        void parseOption(const std::string& option, bool& result);
        void parseOption(const std::string& option, std::string& result);
        void parseOption(const std::string& option, std::vector<std::string>& result);
    };
} // namespace vkBasalt

#endif // CONFIG_HPP_INCLUDED
