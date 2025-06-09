#include <iostream>
#include <vulkan/vulkan.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;

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
        return EXIT_FAILURE;
    }


    return 0;
}