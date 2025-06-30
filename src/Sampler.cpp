//
// Created by rfdic on 6/27/2025.
//

#include "Sampler.h"

namespace reactor {

 Sampler::Sampler(vk::Device device, const vk::SamplerCreateInfo &createInfo)
: m_device(device) {
     m_sampler = m_device.createSampler(createInfo);
 }

 Sampler::~Sampler() {
     if (m_sampler) {
         m_device.destroySampler(m_sampler, nullptr);
     }
 }


} // reactor