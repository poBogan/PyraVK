cmake_minimum_required(VERSION 3.24)
project(PyraLib LANGUAGES CXX)

include(FetchContent)


set(PYRA_INCLUDE_PATH "../include") 
file(GLOB_RECURSE PYRA_HEADER_FILES "../include/PyraVK/*.h")
file(GLOB_RECURSE PYRA_SOURCE_FILES "../source/*.cpp")

find_package(Vulkan REQUIRED)

FetchContent_Declare(
	spdlog 
	GIT_REPOSITORY "https://github.com/gabime/spdlog"
	GIT_TAG "v1.17.0"
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
	glm
	GIT_REPOSITORY "https://github.com/g-truc/glm"
	GIT_TAG "1.0.3"
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        vmahpp
        URL      "https://github.com/YaaZ/VulkanMemoryAllocator-Hpp/releases/download/v3.4.0%2B2/VulkanMemoryAllocator-Hpp-3.4.0.tar.gz"
        URL_HASH "SHA256=588e4aa20898dedca99e72d1bfa428303d3076102108884b8b6a178195404bc9"
)
FetchContent_MakeAvailable(vmahpp)

add_library(pyra_static STATIC ${PYRA_HEADER_FILES} ${PYRA_SOURCE_FILES})
add_library(PyraVK::pyra_static ALIAS pyra_static)

target_compile_features(pyra_static PUBLIC cxx_std_20)

target_include_directories(pyra_static PUBLIC ${PYRA_INCLUDE_PATH})


target_link_libraries(pyra_static 
PUBLIC Vulkan::Vulkan 
PUBLIC spdlog::spdlog
PUBLIC glm::glm
PUBLIC VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
)