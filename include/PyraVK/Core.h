#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

#define pyra_assert(assertion)\
if(!(assertion)) {\
    spdlog::critical("[{:}/{:}() line {:}] -> \"{:}\" failed!",__FILE_NAME__,__FUNCTION__,__LINE__,#assertion);\
    throw std::runtime_error(#assertion " failed");\
}