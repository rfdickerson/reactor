#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include <functional>
#include <memory>

namespace reactor {
class Buffer;

/// @brief Manages Vulkan memory allocations and associated resources.
///   Non-copyable, responsible for buffer creation and allocation.
class Allocator {
public:
  // @brief Constructs an Allocator for Vulkan resource management
  Allocator(vk::PhysicalDevice physicalDevice,
            vk::Device device,
            vk::Instance instance,
            vk::Queue graphicsQueue,
            uint32_t graphicsQueueFamilyIndex);

  ~Allocator();

  Allocator(const Allocator&) = delete;
  Allocator& operator=(const Allocator&) = delete;
  Allocator(Allocator&&) = delete;
  Allocator& operator=(Allocator&&) = delete;

  [[nodiscard]] VmaAllocator getAllocator() const {
    return m_allocator;
  }

  [[nodiscard]] vk::Device getDevice() const {
    return m_device;
  }

  /// Return the graphics queue used for immediate submissions.
  [[nodiscard]] vk::Queue getGraphicsQueue() const {
    return m_graphicsQueue;
  }

  [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const {
    return m_graphicQueueFamilyIndex;
  }

  /// @brief Creates a buffer with given data and usage.
  std::unique_ptr<Buffer> createBufferWithData(
      const void* data,
      vk::DeviceSize size,
      vk::BufferUsageFlags usage,
      const std::string& name = "");

private:
  void immediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function);

  VmaAllocator m_allocator = nullptr;
  vk::Device m_device{};
  vk::Queue m_graphicsQueue{};
  uint32_t m_graphicQueueFamilyIndex{};
};
} // namespace reactor