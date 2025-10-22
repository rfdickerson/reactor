//
// Created by rfdic on 6/9/2025.
//

#ifndef FRAMEMANAGER_HPP
#define FRAMEMANAGER_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.h"

namespace reactor {
    // Using the Frame struct from our previous discussion
    struct Frame {
        vk::CommandBuffer commandBuffer;
        uint64_t timelineValue = 0;
        vk::DescriptorSet cameraDescriptorSet;
        std::unique_ptr<Buffer> uniformBuffer;
    };

    class FrameManager {
    public:
        FrameManager(
            vk::Device device,
            Allocator& allocator,
            uint32_t commandQueueFamilyIndex,
            size_t maxFramesInFlight,
            uint32_t swapchainImageCount);

        ~FrameManager();

        // Methods to orchestrate the frame lifecycle
        bool beginFrame(vk::SwapchainKHR swapchain, uint32_t& outImageIndex); // Handles wait/acquire
        void endFrame(vk::Queue graphicsQueue, vk::Queue presentQueue, vk::SwapchainKHR swapchain, uint32_t imageIndex); // Handles submit/present

        Frame& getCurrentFrame() { return m_frames[m_currentFrame]; }
        size_t getFrameIndex() const { return m_currentFrame; }
        size_t getFramesInFlightCount() const { return m_framesInFlightCount; }
        size_t getCurrentFrameIndex() const { return m_currentFrame; }

    private:
        vk::Device m_device;
        vk::CommandPool m_commandPool;
        std::vector<Frame> m_frames;
        std::vector<vk::Semaphore> m_imageAvailableSemaphores;
        std::vector<vk::Semaphore> m_renderFinishedSemaphores;
        vk::Semaphore m_renderTimelineSemaphore;
        std::vector<uint64_t> m_imagesInFlight;
        uint64_t m_nextTimelineValue = 0;
        size_t m_currentFrame;
        size_t m_framesInFlightCount;
    };
}
#endif //FRAMEMANAGER_HPP
