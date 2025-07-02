
#include "../vulkan/VulkanContext.hpp"
#include "../vulkan/VulkanRenderer.hpp"
#include "Window.hpp"

int main() {

    reactor::RendererConfig config;
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.windowTitle = "Reactor";
    config.vertShaderPath = "../resources/shaders/triangle.vert.spv";
    config.fragShaderPath = "../resources/shaders/triangle.frag.spv";
    config.compositeVertShaderPath = "../resources/shaders/composite.vert.spv";
    config.compositeFragShaderPath = "../resources/shaders/composite.frag.spv";

    reactor::VulkanRenderer renderer(config);
    renderer.run();
    return 0;
}
