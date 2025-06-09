//
// Created by Robert F. Dickerson on 6/8/25.
//

#include "vulkan_context.hpp"

namespace reactor {

VulkanContext::VulkanContext() {
    std::cout << "Creating Vulkan context" << std::endl;
    createInstance();

}

VulkanContext::~VulkanContext() {
    m_instance.destroy();
}

void VulkanContext::createInstance() {
    vk::ApplicationInfo appInfo {
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
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

    try {
        vk::Instance instance = vk::createInstance(createInfo);
        std::cout << "Vulkan instance created successfully." << std::endl;
        instance.destroy();
    } catch (const vk::SystemError& err) {
        std::cerr << "Failed to create Vulkan instance: " << err.what() << std::endl;
        throw err;
    }
}


} // reactor