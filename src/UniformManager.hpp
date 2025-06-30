#pragma once
#include "Allocator.hpp"
#include "Buffer.hpp"

#include <typeindex>

namespace reactor {

class UniformManager {
public:
    UniformManager(Allocator& allocator, size_t framesInFlight);
    ~UniformManager() = default;

    // Register a new type of UBO. This creates the buffers
    template <typename T>
    void registerUBO(const std::string& name) {
        m_uniformBuffers[name] = createFrameSpecificBuffers(sizeof(T));
    }

    // Update the buffer for a specific UBO type on the current frame.
    template <typename T>
    void update(size_t frameIndex, const T& data) {
        const auto& name = m_uboTypeMap.at(std::type_index(typeid(T)));
        auto& buffer = m_uniformBuffers.at(name)[frameIndex];

        void* mappedData = nullptr;
        vmaMapMemory(m_allocator.get(), buffer->allocation(), &mappedData);
        memcpy(mappedData, &data, sizeof(T));
        vmaUnmapMemory(m_allocator.get(), buffer->allocation());
    }

    // Get the descriptor info need to update a descriptor set.
    template <typename T>
    vk::DescriptorBufferInfo getDescriptorInfo(size_t frameIndex) const {
        const auto& name = m_uboTypeMap.at(std::type_index(typeid(T)));
        const auto& buffer = m_uniformBuffers.at(name)[frameIndex];

        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = buffer->size();
        return bufferInfo;
    }

private:
    std::vector<std::unique_ptr<Buffer>> createFrameSpecificBuffers(vk::DeviceSize size);

    Allocator& m_allocator;
    size_t m_framesInFlight;

    // maps a string name to a vector of buffers (one for each frame of flight)
    std::unordered_map<std::string, std::vector<std::unique_ptr<Buffer>>> m_uniformBuffers;

    // maps a C++ type to its registered string name for easy lookup
    std::unordered_map<std::type_index, std::string> m_uboTypeMap;

};

} // reactor

