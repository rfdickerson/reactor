#pragma once

#include <vulkan/vulkan.hpp>

namespace reactor
{

class Sampler
{
public:
    Sampler(vk::Device device, const vk::SamplerCreateInfo& createInfo);
    ~Sampler();

    [[nodiscard]] vk::Sampler get() const
    {
        return m_sampler;
    }

private:
    vk::Device m_device;
    vk::Sampler m_sampler;
};

} // namespace reactor
