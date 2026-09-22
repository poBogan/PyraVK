cmake_minimum_required(VERSION 3.24)
project(PyraExamples LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY "https://github.com/glfw/glfw"
	GIT_TAG "3.5.1"
) 

FetchContent_MakeAvailable(glfw) 

add_executable(example_program main.cpp)

target_link_libraries(example_program
PUBLIC PyraVK::pyra_static
PUBLIC glfw
)