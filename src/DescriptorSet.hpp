//
// Created by rfdic on 6/13/2025.
//

#ifndef DESCRIPTORSET_HPP
#define DESCRIPTORSET_HPP

namespace reactor {
    class Buffer;

    class DescriptorSet {
public:
    DescriptorSet(
        vk::Device device,
        size_t framesInFlight,
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

    ~DescriptorSet();

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const { return m_layout; }
    [[nodiscard]] vk::DescriptorSet get(size_t frame) const { return m_sets[frame]; }
    [[nodiscard]] size_t getFrameCount() const { return m_sets.size(); }

    [[nodiscard]] vk::DescriptorSet getCurrentSet(size_t currentFrame) const { return m_sets[currentFrame]; }

    // helper for updating sets
    void updateSet(const std::vector<vk::WriteDescriptorSet>& writes);

    void updateUniformBuffer(size_t frame, const Buffer& buffer);

private:
    vk::Device m_device;
    vk::DescriptorPool m_pool;
    vk::DescriptorSetLayout m_layout;

    std::vector<vk::DescriptorSet> m_sets;
};

} // reactor

#endif //DESCRIPTORSET_HPP
