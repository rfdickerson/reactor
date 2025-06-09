//
// Created by Robert F. Dickerson on 6/8/25.
//

#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>

namespace reactor {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanContext {
public:
    VulkanContext(GLFWwindow* window);

    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    [[nodiscard]] vk::Instance instance() const { return m_instance; }
    [[nodiscard]] vk::PhysicalDevice physicalDevice() const { return m_physicalDevice; }
    vk::Device device() const { return m_device; }
    vk::SurfaceKHR surface() const { return m_surface; }
    vk::Queue graphicsQueue() const { return m_graphicsQueue; }
    vk::Queue presentQueue() const { return m_presentQueue; }
    QueueFamilyIndices queueFamilies() const { return m_queueFamilies; }

private:
    // Private helper methods to keep the constructor clean
    void createInstance();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool isDeviceSuitable(vk::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

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

#endif //VULKAN_CONTEXT_HPP
