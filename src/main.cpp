
#include "VulkanContext.hpp"
#include "VulkanRenderer.hpp"
#include "Window.hpp"

int main() {

    reactor::RendererConfig config;
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.windowTitle = "Reactor";
    config.vertShaderPath = "../shaders/triangle.vert.spv";
    config.fragShaderPath = "../shaders/triangle.frag.spv";
    config.compositeVertShaderPath = "../shaders/composite.vert.spv";
    config.compositeFragShaderPath = "../shaders/composite.frag.spv";

    reactor::VulkanRenderer renderer(config);
    renderer.run();
    return 0;
}
