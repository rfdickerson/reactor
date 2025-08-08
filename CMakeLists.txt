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
find_package(assimp CONFIG REQUIRED)

add_library(ReactorLib STATIC
        src/vulkan/VulkanContext.cpp
        src/vulkan/VulkanContext.hpp
        src/pch.hpp
        src/vulkan/VulkanRenderer.hpp
        src/vulkan/VulkanRenderer.cpp
        src/core/Window.cpp
        src/core/Window.hpp
        src/vulkan/Swapchain.hpp
        src/vulkan/FrameManager.hpp
        src/vulkan/Swapchain.cpp
        src/vulkan/FrameManager.cpp
        src/vulkan/Pipeline.cpp
        src/vulkan/Pipeline.hpp
        src/vulkan/DescriptorSet.cpp
        src/vulkan/DescriptorSet.hpp
        src/vulkan/Allocator.cpp
        src/vulkan/Allocator.hpp
        src/vulkan/vk_mem_alloc.cpp
        src/vulkan/Buffer.cpp
        src/vulkan/Buffer.hpp
        src/imgui/Imgui.cpp
        src/imgui/Imgui.hpp
        src/vulkan/VulkanUtils.hpp
        src/vulkan/Image.cpp
        src/vulkan/Image.hpp
        src/vulkan/Sampler.cpp
        src/vulkan/Sampler.hpp
        src/vulkan/ImageStateTracker.cpp
        src/vulkan/ImageStateTracker.h
        src/core/Uniforms.hpp
        src/vulkan/UniformManager.cpp
        src/vulkan/UniformManager.hpp
        src/vulkan/ShaderModule.cpp
        src/vulkan/ShaderModule.hpp
        src/core/EventManager.hpp
        src/core/EventManager.cpp
        src/core/Camera.cpp
        src/core/Camera.hpp
        src/core/OrbitController.cpp
        src/core/OrbitController.hpp
        src/core/Application.cpp
        src/core/Application.hpp
        src/vulkan/Vertex.hpp
        src/vulkan/ImageUtils.hpp
        src/vulkan/ImageUtils.cpp
        src/vulkan/Mesh.cpp
        src/vulkan/Mesh.hpp
        src/vulkan/MeshGenerators.hpp
        src/vulkan/MeshGenerators.cpp
        src/core/ModelIO.hpp
        src/core/ModelIO.cpp
        src/vulkan/ShadowMapping.hpp
        src/vulkan/ShadowMapping.cpp
        src/logging/Logger.hpp
        src/logging/SpdlogSink.cpp
        src/logging/SpdlogSink.hpp
        src/logging/ImGuiConsoleSink.cpp
        src/logging/ImGuiConsoleSink.hpp
        src/vulkan/DebugUtils.hpp
)

target_compile_definitions(ReactorLib PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(ReactorLib PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(ReactorLib PUBLIC
        Vulkan::Vulkan
        Vulkan::Headers
        glfw
        fmt::fmt
        spdlog::spdlog
        GPUOpen::VulkanMemoryAllocator
        glm::glm
        imgui::imgui
        assimp::assimp
)

target_precompile_headers(ReactorLib PRIVATE src/pch.hpp)

add_executable(Editor src/core/main.cpp)
target_link_libraries(Editor PRIVATE ReactorLib)

add_executable(BuildModel src/tools/ModelBuilder.cpp)
target_link_libraries(BuildModel PRIVATE ReactorLib)
