#pragma once
#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <sstream>
#include <vector>

namespace VulkanFX
{
    void addUniqueCString(std::vector<const char*>& stringVector, const char* addString);

    enum class Color
    {
        defaultColor,

        black,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white
    };

    void outputInColor(std::string output, Color foreground = Color::defaultColor, Color background = Color::defaultColor);

    template<typename T>
    auto convertToString(T object) -> std::string
    {
        std::stringstream ss;
        ss << object;
        return ss.str();
    }
} // namespace VulkanFX

#endif // UTIL_HPP_INCLUDED
