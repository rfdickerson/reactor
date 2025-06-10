//
// Created by rfdic on 6/9/2025.
//

#ifndef FRAMEMANAGER_HPP
#define FRAMEMANAGER_HPP

#include <vulkan/vulkan.hpp>

namespace reactor {
    // Using the Frame struct from our previous discussion
    struct Frame {
        vk::CommandBuffer commandBuffer;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        vk::Fence inFlightFence;
    };

    class FrameManager {
    public:
        FrameManager(vk::Device device, uint32_t commandQueueFamilyIndex, size_t maxFramesInFlight);
        ~FrameManager();

        // Methods to orchestrate the frame lifecycle
        bool beginFrame(vk::SwapchainKHR swapchain, uint32_t& outImageIndex); // Handles wait/acquire
        void endFrame(vk::Queue graphicsQueue, vk::Queue presentQueue, vk::SwapchainKHR swapchain, uint32_t imageIndex); // Handles submit/present

        Frame& getCurrentFrame() { return m_frames[m_currentFrame]; }

    private:
        vk::Device m_device;
        vk::CommandPool m_commandPool;
        std::vector<Frame> m_frames;
        size_t m_currentFrame = 0;
    };
}
#endif //FRAMEMANAGER_HPP
