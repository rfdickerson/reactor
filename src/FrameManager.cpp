#include "FrameManager.hpp"

#include <stdexcept>
#include <vector>

namespace reactor {

FrameManager::FrameManager(vk::Device device, uint32_t commandQueueFamilyIndex, size_t maxFramesInFlight, uint32_t swapchainImageCount)
    : m_device(device), m_currentFrame(0)
{
    spdlog::info("Creating FrameManager with {} frames in flight", maxFramesInFlight);

    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        commandQueueFamilyIndex
    };
    m_commandPool = m_device.createCommandPool(poolInfo);

    // Allocate per-frame resources
    m_frames.resize(maxFramesInFlight);
    for (auto& frame : m_frames) {
        vk::CommandBufferAllocateInfo allocInfo{
            m_commandPool,
            vk::CommandBufferLevel::ePrimary,
            1
        };
        frame.commandBuffer = m_device.allocateCommandBuffers(allocInfo)[0];
        vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
        frame.inFlightFence = m_device.createFence(fenceInfo);
    }

    vk::SemaphoreCreateInfo semaphoreInfo{};

    m_imageAvailableSemaphores.resize(maxFramesInFlight);
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        m_imageAvailableSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
    }

    m_renderFinishedSemaphores.resize(swapchainImageCount);
    for (size_t i = 0; i < swapchainImageCount; i++) {
        m_renderFinishedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
    }

    m_imagesInFlight.resize(swapchainImageCount, VK_NULL_HANDLE);

}

FrameManager::~FrameManager() {
    spdlog::info("Destroying FrameManager and cleaning up resources.");

    for (auto& semaphore : m_imageAvailableSemaphores) {
        m_device.destroySemaphore(semaphore);
    }

    for (auto& semaphore : m_renderFinishedSemaphores) {
        m_device.destroySemaphore(semaphore);
    }

    for (auto& frame : m_frames) {
        if (frame.inFlightFence) {
            m_device.destroyFence(frame.inFlightFence);
        }
    }

    if (m_commandPool) {
        m_device.destroyCommandPool(m_commandPool);
    }
}

bool FrameManager::beginFrame(vk::SwapchainKHR swapchain, uint32_t& outImageIndex) {
   vk::Result result = m_device.waitForFences(m_frames[m_currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    auto resultValue = m_device.acquireNextImageKHR(
        swapchain,
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE
        );

    if (resultValue.result == vk::Result::eErrorOutOfDateKHR) {
        return false;
    }
   if (resultValue.result != vk::Result::eSuccess && resultValue.result != vk::Result::eSuboptimalKHR) {
       throw std::runtime_error("Failed to acquire swapchain image!");
   }

   outImageIndex = resultValue.value;

    if (m_imagesInFlight[outImageIndex] != VK_NULL_HANDLE) {
        m_device.waitForFences(m_imagesInFlight[outImageIndex], VK_TRUE, UINT64_MAX);
    }

    // Associate the current frame's fence with the aquired swapchain image
    m_imagesInFlight[outImageIndex] = m_frames[m_currentFrame].inFlightFence;

    m_device.resetFences(m_frames[m_currentFrame].inFlightFence);

    return true;
}

void FrameManager::endFrame(vk::Queue graphicsQueue, vk::Queue presentQueue, vk::SwapchainKHR swapchain, uint32_t imageIndex) {
    Frame& frame = m_frames[m_currentFrame];

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submitInfo{
        1, &m_imageAvailableSemaphores[m_currentFrame],
        &waitStages,
        1, &frame.commandBuffer,
        1, &m_renderFinishedSemaphores[imageIndex]
    };

    graphicsQueue.submit(submitInfo, frame.inFlightFence);

    vk::PresentInfoKHR presentInfo{
        1, &m_renderFinishedSemaphores[imageIndex],
        1, &swapchain,
        &imageIndex
    };

    vk::Result result = presentQueue.presentKHR(presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        // Handle swapchain recreation here
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swapchain image!");
    }

    // Advance to next frame
    m_currentFrame = (m_currentFrame + 1) % m_frames.size();
}

} // namespace reactor
