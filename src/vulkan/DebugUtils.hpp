#pragma once

#include <vulkan/vulkan.hpp>
#include <string>

#ifndef NDEBUG
    const bool g_enableDebugMarkers = true;
#else
const bool g_enableDebugMarkers = false;
#endif

namespace reactor::Debug
{

// Sets the name of any Vulkan object
inline void setObjectName(vk::Device device, uint64_t object, vk::ObjectType objectType, const std::string& name) {
    if (!g_enableDebugMarkers) return;

    vk::DebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = object;
    nameInfo.pObjectName = name.c_str();

    // No need for dynamic dispatch here, as device functions are loaded with the device
    device.setDebugUtilsObjectNameEXT(nameInfo);
}

// Begins a labeled region in a command buffer
inline void beginLabel(vk::CommandBuffer cmd, const std::string& label, const std::array<float, 4>& color = {1.0f, 1.0f, 1.0f, 1.0f}) {
    if (!g_enableDebugMarkers) return;

    vk::DebugUtilsLabelEXT labelInfo{};
    labelInfo.pLabelName = label.c_str();
    labelInfo.color[0] = color[0];
    labelInfo.color[1] = color[1];
    labelInfo.color[2] = color[2];
    labelInfo.color[3] = color[3];

    cmd.beginDebugUtilsLabelEXT(labelInfo);
}

// Ends a labeled region in a command buffer
inline void endLabel(vk::CommandBuffer cmd) {
    if (!g_enableDebugMarkers) return;
    cmd.endDebugUtilsLabelEXT();
}

} // namespace reactor::Debug