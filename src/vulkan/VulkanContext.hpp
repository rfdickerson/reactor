#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>

namespace reactor {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanContext {
public:
    explicit VulkanContext(GLFWwindow* window);

    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    [[nodiscard]] vk::Instance instance() const { return m_instance; }
    [[nodiscard]] vk::PhysicalDevice physicalDevice() const { return m_physicalDevice; }
    [[nodiscard]] vk::Device device() const { return m_device; }
    [[nodiscard]] vk::SurfaceKHR surface() const { return m_surface; }
    [[nodiscard]] vk::Queue graphicsQueue() const { return m_graphicsQueue; }
    [[nodiscard]] vk::Queue presentQueue() const { return m_presentQueue; }
    [[nodiscard]] QueueFamilyIndices queueFamilies() const { return m_queueFamilies; }

private:
    // Private helper methods to keep the constructor clean
    void createInstance();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool isDeviceSuitable(vk::PhysicalDevice device);
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) const;

    // Member variables - these are owned by the context
    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;

    // Queues are retrieved from the logical device
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    QueueFamilyIndices m_queueFamilies;

};

} // reactor

