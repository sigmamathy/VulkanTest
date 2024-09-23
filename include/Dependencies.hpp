#ifndef DEPENDENCIES_HPP
#define DEPENDENCIES_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <array>
#include <vector>
#include <cstring>
#include <algorithm>
#include <unordered_set>
#include <limits>
#include <fstream>

#define ERRCHECK(...) if (!(__VA_ARGS__)) [[unlikely]] { std::cerr << "Requirement: " #__VA_ARGS__\
    " FAILED. (" THISFILE ":" << std::to_string(__LINE__) <<")\n"; throw std::runtime_error(#__VA_ARGS__); }

#endif //DEPENDENCIES_HPP
