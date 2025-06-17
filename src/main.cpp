
#include "VulkanContext.hpp"
#include "VulkanRenderer.hpp"
#include "Window.hpp"

int main() {

    reactor::RendererConfig config;
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.windowTitle = "Reactor";
    config.vertShaderPath = "shaders/vert.spv";
    config.fragShaderPath = "shaders/frag.spv";

    reactor::VulkanRenderer renderer(config);
    renderer.run();
    return 0;
}
