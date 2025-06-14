//
// Created by rfdic on 6/13/2025.
//

#include "Allocator.hpp"

#include <spdlog/spdlog.h>

namespace reactor {

    Allocator::Allocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance) {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device         = device;
        allocatorInfo.instance       = instance;

        vmaCreateAllocator(&allocatorInfo, &m_allocator);

        spdlog::info("Allocator created");
    }

    Allocator::~Allocator() {
        if (m_allocator) vmaDestroyAllocator(m_allocator);
    }


} // reactor