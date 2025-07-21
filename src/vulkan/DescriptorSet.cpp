//
// Created by rfdic on 6/13/2025.
//

#include "DescriptorSet.hpp"

#include "Buffer.hpp"

namespace reactor {

    DescriptorSet::DescriptorSet(vk::Device device, size_t framesInFlight, const std::vector<vk::DescriptorSetLayoutBinding> &bindings)
        : m_device(device)
    {
        // create a descriptor set layout
        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);
        m_layout = device.createDescriptorSetLayout(layoutInfo);

        std::vector<vk::DescriptorPoolSize> poolSizes;
        for (const auto& binding : bindings) {
            poolSizes.push_back({binding.descriptorType, static_cast<uint32_t>(framesInFlight)});
        }

        vk::DescriptorPoolCreateInfo poolInfo(
            vk::DescriptorPoolCreateFlags(),
            static_cast<uint32_t>(framesInFlight),
            static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

        m_pool = device.createDescriptorPool(poolInfo);

        // allocate one set per frame
        std::vector<vk::DescriptorSetLayout> layouts(framesInFlight, m_layout);
        vk::DescriptorSetAllocateInfo allocInfo(m_pool, layouts);
        m_sets = device.allocateDescriptorSets(allocInfo);

    }

    DescriptorSet::~DescriptorSet() {
        if (m_pool) m_device.destroyDescriptorPool(m_pool);
        if (m_layout) m_device.destroyDescriptorSetLayout(m_layout);
    }

    void DescriptorSet::updateUniformBuffer(size_t frame, const Buffer &buffer) {
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = buffer.getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = buffer.size();

        vk::WriteDescriptorSet write;
        write.dstSet = m_sets[frame];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = vk::DescriptorType::eUniformBuffer;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;
        write.pImageInfo = nullptr;
        write.pTexelBufferView = nullptr;

        m_device.updateDescriptorSets(1, &write, 0, nullptr);
    }

void DescriptorSet::updateSet(const std::vector<vk::WriteDescriptorSet> &writes) {
        m_device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
} // reactor