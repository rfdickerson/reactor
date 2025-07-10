#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <array>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription bindingDescription() {
        vk::VertexInputBindingDescription desc{};
        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = vk::VertexInputRate::eVertex;
        return desc;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attrs{};
        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = vk::Format::eR32G32B32Sfloat;
        attrs[0].offset = offsetof(Vertex, pos);

        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = vk::Format::eR32G32B32Sfloat;
        attrs[1].offset = offsetof(Vertex, color);
        return attrs;
    }
};
