#pragma once 
#include <spdlog/spdlog.h>
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
if(!(assertion)) {spdlog::critical("\nFile: {:}\nFunction: {:}\nLine: {:}\nFailed Assertion: {:}",__FILE__,__FUNCTION__,__LINE__,#assertion); \
throw std::runtime_error(#assertion " failed");}
