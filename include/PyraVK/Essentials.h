#pragma once 
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include <vector>
#include <queue>
#include <string>
#include <map>
#include <tuple>
#include <unordered_map>
#include <memory>
#include <exception>
#include <optional>
#include <ranges>
#include <algorithm>

#define pyra_assert(assertion) \
if(!(assertion)) {spdlog::critical("Function: {:} => Failed Assertion: {:}",__FUNCTION__,#assertion); \
throw std::runtime_error(#assertion " failed");}

#define pyra_error(message,...) spdlog::error("Function: {:} => " message,__func__)
