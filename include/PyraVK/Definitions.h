#pragma once 
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
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
if(!(assertion)) {spdlog::critical(fail_message,__VA_ARGS__); \
throw std::runtime_error(fail_message);}

#define pyra_warn(condition,...) \
if(condition) spdlog::warn(__VA_ARGS__)

#define pyra_info(condition,...)\
if(condition) spdlog::info(__VA_ARGS__)

namespace pyra
{
	template<class T> 
	std::size_t combineHash(std::size_t prev,const T curr)
	{
		std::hash<T> tHasher;
		std::size_t res = prev ^ (tHasher(curr) + 0x9e3779b9 + (prev << 6) + (prev >> 2));
		return res;
	}

	template<class...Types> 
	std::size_t chainHash(const Types&...args)
	{
		std::size_t seed = 0;
		((seed = combineHash(seed, args)), ...); 
		return seed;
	}

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