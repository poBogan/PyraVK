#pragma once 
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <string>
#include <memory>
#include <exception>
#include <ranges>
#include <algorithm>


#define pyra_log_info(...)		if(pyra::console_logger) spdlog::info(__VA_ARGS__)
#define pyra_log_warning(...)	if(pyra::console_logger) spdlog::warn(__FUNCTION__ ": " __VA_ARGS__)
#define pyra_log_error(...)		if(pyra::console_logger) spdlog::error(__FUNCTION__ ": " __VA_ARGS__)
#define pyra_log_critical(...)  if(pyra::console_logger) spdlog::critical(__FUNCTION__ ": " __VA_ARGS__)

namespace pyra
{

	static bool console_logger = true; 

}