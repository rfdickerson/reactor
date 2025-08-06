#include "VulkanContext.hpp"

#include <set>

#include "../logging/Logger.hpp"

namespace {
void printVulkanVersion() {
    uint32_t vulkanApiVersion = 0;
    if (vk::enumerateInstanceVersion(&vulkanApiVersion) == vk::Result::eSuccess) {
        uint32_t major = VK_VERSION_MAJOR(vulkanApiVersion);
        uint32_t minor = VK_VERSION_MINOR(vulkanApiVersion);
        uint32_t patch = VK_VERSION_PATCH(vulkanApiVersion);
        LOG_INFO("Vulkan API version: {}.{}.{}", major, minor, patch);
    } else {
        spdlog::error("Failed to enumerate Vulkan API version");
    }
}
}

namespace reactor {

VulkanContext::VulkanContext(GLFWwindow* window) {
    spdlog::info("Creating Vulkan context");
    createInstance();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();

}

VulkanContext::~VulkanContext() {
    m_device.destroy();
    m_instance.destroySurfaceKHR(m_surface);
    m_instance.destroy();
}

void VulkanContext::createInstance() {
    constexpr vk::ApplicationInfo appInfo {
        "Reactor App",
        1,
        "No Engine",
        1,
        VK_API_VERSION_1_3
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // add the GLFW extensions

    #ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    #endif

    vk::InstanceCreateInfo createInfo {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.setPEnabledExtensionNames(extensions);

    #ifdef __APPLE__
    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    #endif

    try {
        m_instance = vk::createInstance(createInfo);
        spdlog::info("Vulkan instance created");
        printVulkanVersion();

    } catch (const vk::SystemError& err) {
        std::cerr << "Failed to create Vulkan instance: " << err.what() << std::endl;
        throw;
    }
}

void VulkanContext::createSurface(GLFWwindow *window) {
    if (glfwCreateWindowSurface(m_instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface)
) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

}

    void VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    if (m_instance.enumeratePhysicalDevices(&deviceCount, nullptr) != vk::Result::eSuccess || deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<vk::PhysicalDevice> devices(deviceCount);
    auto result = m_instance.enumeratePhysicalDevices(&deviceCount, devices.data());
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to enumerate GPUs with Vulkan support!");
    }

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
            spdlog::info("Selected physical device: {}", static_cast<const char*>(properties.deviceName));

            return;
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU!");
}

    bool VulkanContext::isDeviceSuitable(vk::PhysicalDevice device) {
    const QueueFamilyIndices indices = findQueueFamilies(device);

    // Basic check for device suitability: does it have a graphics and present queue?
    // You'll likely want to add more checks here, e.g., device extensions, swap chain support.
    return indices.isComplete();
}

QueueFamilyIndices VulkanContext::findQueueFamilies(vk::PhysicalDevice device) const {
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        vk::Bool32 presentSupport = vk::False;
        if (const auto result = device.getSurfaceSupportKHR(i, m_surface, &presentSupport);
            result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get surface support!");
        }

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void VulkanContext::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{}; // No special features for now

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext = &dynamicRenderingFeatures;

    #ifdef __APPLE__
    // Required for MoltenVK
    std::vector<const char*> deviceExtensions = { "VK_KHR_portability_subset" };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    #endif

    try {
        m_device = m_physicalDevice.createDevice(createInfo);
        m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
        m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);
        m_queueFamilies = indices; // Store the found queue families

        spdlog::info("Logical device created");
    } catch (const vk::SystemError& err) {
        std::cerr << "Failed to create logical device: " << err.what() << std::endl;
        throw;
    }
}


} // reactor