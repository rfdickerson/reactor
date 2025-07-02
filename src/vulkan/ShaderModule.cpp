//
// Created by rfdic on 7/2/2025.
//

#include "ShaderModule.hpp"

namespace reactor {

 ShaderModule::ShaderModule(vk::Device device, const std::vector<char> &code)
     : m_device(device) {
     vk::ShaderModuleCreateInfo createInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
     m_handle = m_device.createShaderModule(createInfo);
 }

 ShaderModule::~ShaderModule() {
     if (m_handle) m_device.destroyShaderModule(m_handle);
 }



} // reactor