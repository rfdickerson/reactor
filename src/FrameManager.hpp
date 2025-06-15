//
// Created by rfdic on 6/9/2025.
//

#ifndef FRAMEMANAGER_HPP
#define FRAMEMANAGER_HPP

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

namespace reactor {
    // Using the Frame struct from our previous discussion
    struct Frame {
        vk::CommandBuffer commandBuffer;
        vk::Fence inFlightFence;
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

    private:
        vk::Device m_device;
        vk::CommandPool m_commandPool;
        std::vector<Frame> m_frames;
        std::vector<vk::Semaphore> m_imageAvailableSemaphores;
        std::vector<vk::Semaphore> m_renderFinishedSemaphores;
        std::vector<vk::Fence> m_imagesInFlight;
        size_t m_currentFrame;
    };
}
#endif //FRAMEMANAGER_HPP
