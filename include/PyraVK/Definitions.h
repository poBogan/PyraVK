#pragma once 
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <exception>
#include <optional>
#include <ranges>
#include <algorithm>

#define pyra_assert(assertion,fail_message,...) \
if(!(assertion)) {spdlog::error(fail_message,__VA_ARGS__); \
throw std::runtime_error(fail_message);}

#define pyra_warn(condition,...) \
if(condition) spdlog::warn(__VA_ARGS__);

namespace pyra
{
	class Base
	{
	protected : 
		Base() = default;
		~Base() = default;

		Base(const Base&) = delete;
		Base& operator=(const Base&) = delete; 

		Base(Base&&) = default;
		Base& operator=(Base&&) = default;
	};

}