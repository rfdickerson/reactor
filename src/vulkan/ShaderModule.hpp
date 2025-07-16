#pragma once
#include <vulkan/vulkan.hpp>

namespace reactor
{

class ShaderModule
{
public:
    ShaderModule(vk::Device device, const std::vector<char>& code);
    ~ShaderModule();

    [[nodiscard]] vk::ShaderModule getHandle() const
    {
        return m_handle;
    };

    // disallow copy, allow move
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&& other) noexcept
        : m_handle(other.m_handle)
    {
        other.m_handle = nullptr;
    }
    ShaderModule& operator=(ShaderModule&&) = delete;

private:
    vk::ShaderModule m_handle;
    vk::Device m_device;
};

} // namespace reactor
