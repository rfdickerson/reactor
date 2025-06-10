#include "FrameManager.hpp"

#include <stdexcept>
#include <vector>

namespace reactor {

FrameManager::FrameManager(vk::Device device, uint32_t commandQueueFamilyIndex, size_t maxFramesInFlight)
    : m_device(device), m_currentFrame(0)
{
    spdlog::info("Creating FrameManager with {} frames in flight", maxFramesInFlight);

    // Create command pool for this manager
    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        commandQueueFamilyIndex
    };
    m_commandPool = m_device.createCommandPool(poolInfo);

    // Allocate per-frame resources
    m_frames.resize(maxFramesInFlight);
    for (auto& frame : m_frames) {
        // Command buffer
        vk::CommandBufferAllocateInfo allocInfo{
            m_commandPool,
            vk::CommandBufferLevel::ePrimary,
            1
        };
        frame.commandBuffer = m_device.allocateCommandBuffers(allocInfo)[0];

        // Semaphores
        vk::SemaphoreCreateInfo semaphoreInfo{};
        frame.imageAvailableSemaphore = m_device.createSemaphore(semaphoreInfo);
        frame.renderFinishedSemaphore = m_device.createSemaphore(semaphoreInfo);

        // Fence (start signaled so we don't wait on first use)
        vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
        frame.inFlightFence = m_device.createFence(fenceInfo);
    }
}

FrameManager::~FrameManager() {
    spdlog::info("Destroying FrameManager and cleaning up resources.");

    for (auto& frame : m_frames) {
        if (frame.commandBuffer) {
            m_device.freeCommandBuffers(m_commandPool, frame.commandBuffer);
        }
        if (frame.imageAvailableSemaphore) {
            m_device.destroySemaphore(frame.imageAvailableSemaphore);
        }
        if (frame.renderFinishedSemaphore) {
            m_device.destroySemaphore(frame.renderFinishedSemaphore);
        }
        if (frame.inFlightFence) {
            m_device.destroyFence(frame.inFlightFence);
        }
    }
    if (m_commandPool) {
        m_device.destroyCommandPool(m_commandPool);
    }
}

bool FrameManager::beginFrame(vk::SwapchainKHR swapchain, uint32_t& outImageIndex) {
    Frame& frame = m_frames[m_currentFrame];

    // Wait for the previous frame to finish
    m_device.waitForFences(frame.inFlightFence, VK_TRUE, UINT64_MAX);

    // Reset the fence for this frame
    m_device.resetFences(frame.inFlightFence);

    // Acquire next swapchain image
    auto result = m_device.acquireNextImageKHR(
        swapchain,
        UINT64_MAX,
        frame.imageAvailableSemaphore,
        nullptr,
        &outImageIndex
    );

    if (result == vk::Result::eErrorOutOfDateKHR) {
        return false; // Resize or swapchain recreation needed
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }
    return true;
}

void FrameManager::endFrame(vk::Queue graphicsQueue, vk::Queue presentQueue, vk::SwapchainKHR swapchain, uint32_t imageIndex) {
    Frame& frame = m_frames[m_currentFrame];

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo{
        1, &frame.imageAvailableSemaphore, &waitStages,
        1, &frame.commandBuffer,
        1, &frame.renderFinishedSemaphore
    };

    // Submit the command buffer
    graphicsQueue.submit(submitInfo, frame.inFlightFence);

    // Present the result
    vk::PresentInfoKHR presentInfo{
        1, &frame.renderFinishedSemaphore,
        1, &swapchain,
        &imageIndex
    };
    presentQueue.presentKHR(presentInfo);

    // Advance to next frame
    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
}

} // namespace reactor
