#pragma once 
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <string>
#include <memory>
#include <exception>
#include <optional>
#include <ranges>
#include <algorithm>

#define pyra_assert(assertion,fail_messsage) \
if(!assertion) {spdlog::error("Exception at -> " __FUNCTION__ " | {:}",fail_messsage); \
throw std::exception(fail_messsage);}


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