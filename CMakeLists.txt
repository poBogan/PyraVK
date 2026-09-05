cmake_minimum_required(VERSION 3.24)
project(PyraVK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PYRA_INCLUDE_PATH "include") 
file(GLOB_RECURSE PYRA_HEADER_FILES "include/PyraVK/*.h")
file(GLOB_RECURSE PYRA_SOURCE_FILES "source/*.cpp")

find_package(Vulkan REQUIRED)

#Static Libray
add_subdirectory("third-party/spdlog")
add_library(pyra STATIC ${PYRA_HEADER_FILES} ${PYRA_SOURCE_FILES})
target_include_directories(pyra PUBLIC ${PYRA_INCLUDE_PATH})
target_link_libraries(pyra PUBLIC Vulkan::Vulkan spdlog::spdlog)

# Example Program
add_subdirectory("examples/glfw")
add_executable(testProgram "examples/main.cpp")
target_link_libraries(testProgram pyra)
target_link_libraries(testProgram glfw)