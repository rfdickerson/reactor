
#include "../vulkan/VulkanContext.hpp"
#include "../vulkan/VulkanRenderer.hpp"
#include "Application.hpp"
#include "Window.hpp"

int main() {

    try {
        reactor::Application app;
        app.run();
    } catch (const std::exception& e) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
