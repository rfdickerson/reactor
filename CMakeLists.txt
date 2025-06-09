cmake_minimum_required(VERSION 3.31)
project(Reactor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG)

add_executable(Reactor src/main.cpp
        src/vulkan_context.cpp
        src/vulkan_context.hpp
        src/pch.hpp)
target_link_libraries(Reactor Vulkan::Vulkan Vulkan::Headers)

target_precompile_headers(Reactor PRIVATE src/pch.hpp)
