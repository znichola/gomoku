#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

template<typename T>
T parseEnum(const std::string& s, const std::unordered_map<std::string, T>& map) {
    auto it = map.find(s);
    if (it != map.end()) return it->second;
    throw std::runtime_error("invalid enum value: " + s);
}

template<typename T>
std::string serializeEnum(T e, const std::unordered_map<T, std::string>& map) {
    auto it = map.find(e);
    if (it != map.end()) return "\"" + it->second + "\"";
    throw std::runtime_error("invalid enum value: " + std::to_string(static_cast<unsigned>(e)));
}

