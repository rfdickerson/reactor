#include "Sampler.hpp"

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