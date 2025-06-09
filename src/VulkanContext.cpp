//
// Created by Robert F. Dickerson on 6/8/25.
//

#include "VulkanContext.hpp"

namespace reactor {

VulkanContext::VulkanContext() {
    std::cout << "Creating Vulkan context" << std::endl;
    createInstance();

}

VulkanContext::~VulkanContext() {
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

    const std::vector extensions = {
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    };

    vk::InstanceCreateInfo createInfo {};
    createInfo.pApplicationInfo = &appInfo;
    // createInfo.enabledExtensionCount = extensions.size();
    // createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.setPEnabledExtensionNames(extensions);

    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

    try {
        m_instance = vk::createInstance(createInfo);
        std::cout << "Vulkan instance created successfully." << std::endl;

        uint32_t vulkanApiVersion;
        if (vk::enumerateInstanceVersion(&vulkanApiVersion) == vk::Result::eSuccess) {
            std::cout << "Vulkan API Version: "
                      << VK_VERSION_MAJOR(vulkanApiVersion) << "."
                      << VK_VERSION_MINOR(vulkanApiVersion) << "."
                      << VK_VERSION_PATCH(vulkanApiVersion) << std::endl;
        } else {
            std::cerr << "Failed to enumerate Vulkan instance version." << std::endl;
        }

    } catch (const vk::SystemError& err) {
        std::cerr << "Failed to create Vulkan instance: " << err.what() << std::endl;
        throw err;
    }
}

void VulkanContext::createSurface(GLFWwindow *window) {
    if (glfwCreateWindowSurface(m_instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface)
) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }


}

} // reactor