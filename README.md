# Reactor

A modern C++ Vulkan renderer built using Vulkan-Hpp, designed with extensibility and clarity in mind. This renderer leverages dynamic rendering, multi-sample anti-aliasing (MSAA), HDR rendering, and a modular render loop architecture to simplify development of real-time graphics applications.

## ✨ Features
- Vulkan-Hpp: Modern C++ bindings for Vulkan for type safety and cleaner API.
- Dynamic Rendering: No need for complex render pass setups.
- MSAA + Resolve: High-quality anti-aliasing using MSAA images resolved into HDR buffers.
- HDR Rendering: Full 16-bit float rendering pipeline using R16G16B16A16_SFLOAT.
- ImGui Integration: Built-in user interface rendering support.
- Robust Frame Management: Supports multiple frames in flight.
- Descriptor Management: Automatic uniform buffer updates and descriptor set handling.
- Swapchain Resilience: Handles window resize events and swapchain recreation cleanly.

## 🚀 Getting Started

**Prerequisites**
- Vulkan SDK (1.3+)
- C++20 compiler (e.g., clang++ or g++)
- CMake (3.18+)
- glfw for windowing
- glm for math
- Vulkan Memory Allocator (VMA) for efficient GPU memory management
- ImGui for UI

### 📜 License

MIT License. See LICENSE for details.
