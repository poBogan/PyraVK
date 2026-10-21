#pragma once 
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <queue>
#include <string>
#include <unordered_set>
#include <map>
#include <tuple>
#include <unordered_map>
#include <memory>
#include <exception>
#include <optional>
#include <ranges>
#include <algorithm>

#define pyra_assert(assertion) \
if(!(assertion)) {\
    spdlog::critical("[{:}/{:}() line {:}] -> \"{:}\" failed!",__FILE_NAME__,__FUNCTION__,__LINE__,#assertion); \
    throw std::runtime_error(#assertion " failed");\
}

#define pyra_error(message,...) spdlog::error("Function: {:} => " message,__func__)
