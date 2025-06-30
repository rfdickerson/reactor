cmake_minimum_required(VERSION 3.28)
project(Reactor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(Reactor src/main.cpp
        src/VulkanContext.cpp
        src/VulkanContext.hpp
        src/pch.hpp
        src/VulkanRenderer.hpp
        src/VulkanRenderer.cpp
        src/Window.cpp
        src/Window.hpp
        src/Swapchain.hpp
        src/FrameManager.hpp
        src/Swapchain.cpp
        src/FrameManager.cpp
        src/Pipeline.cpp
        src/Pipeline.hpp
        src/DescriptorSet.cpp
        src/DescriptorSet.hpp
        src/Allocator.cpp
        src/Allocator.hpp
        src/vk_mem_alloc.cpp
        src/Buffer.cpp
        src/Buffer.hpp
        src/Imgui.cpp
        src/Imgui.hpp
        src/VulkanUtils.hpp
        src/Image.cpp
        src/Image.hpp
        src/Sampler.cpp
        src/Sampler.h
        src/ImageStateTracker.cpp
        src/ImageStateTracker.h
        src/Uniforms.hpp
        src/UniformManager.cpp
        src/UniformManager.hpp)

target_link_libraries(Reactor PRIVATE Vulkan::Vulkan Vulkan::Headers glfw fmt::fmt spdlog::spdlog GPUOpen::VulkanMemoryAllocator glm::glm)
target_link_libraries(Reactor PRIVATE imgui::imgui)

target_precompile_headers(Reactor PRIVATE src/pch.hpp)
