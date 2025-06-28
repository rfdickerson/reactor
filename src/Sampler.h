//
// Created by rfdic on 6/27/2025.
//

#ifndef SAMPLER_H
#define SAMPLER_H
#include "Sampler.h"

#include <vulkan/vulkan.hpp>

namespace reactor {

class Sampler {
public:
    Sampler(vk::Device device, const vk::SamplerCreateInfo& createInfo);
    ~Sampler();

    vk::Sampler get() const { return m_sampler; }

private:
    vk::Device m_device;
    vk::Sampler m_sampler;

};

} // reactor

#endif //SAMPLER_H
