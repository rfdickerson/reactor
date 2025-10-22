#include "FrameManager.hpp"

#include <array>
#include <stdexcept>
#include <vector>

namespace reactor {

FrameManager::FrameManager(vk::Device device, Allocator& allocator, uint32_t commandQueueFamilyIndex, size_t maxFramesInFlight, uint32_t swapchainImageCount)
    : m_device(device), m_currentFrame(0), m_framesInFlightCount(maxFramesInFlight)
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

        // create a uniform buffer
        frame.uniformBuffer = std::make_unique<Buffer>(allocator, 1024, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_ONLY);
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

    vk::SemaphoreTypeCreateInfo timelineCreateInfo{};
    timelineCreateInfo.semaphoreType = vk::SemaphoreType::eTimeline;
    timelineCreateInfo.initialValue = 0;

    vk::SemaphoreCreateInfo timelineSemaphoreInfo{};
    timelineSemaphoreInfo.pNext = &timelineCreateInfo;

    m_renderTimelineSemaphore = m_device.createSemaphore(timelineSemaphoreInfo);

    m_imagesInFlight.resize(swapchainImageCount, 0);

}

FrameManager::~FrameManager() {
    spdlog::info("Destroying FrameManager and cleaning up resources.");

    for (auto& semaphore : m_imageAvailableSemaphores) {
        m_device.destroySemaphore(semaphore);
    }

    for (auto& semaphore : m_renderFinishedSemaphores) {
        m_device.destroySemaphore(semaphore);
    }

    if (m_renderTimelineSemaphore) {
        m_device.destroySemaphore(m_renderTimelineSemaphore);
    }

    if (m_commandPool) {
        m_device.destroyCommandPool(m_commandPool);
    }
}

bool FrameManager::beginFrame(vk::SwapchainKHR swapchain, uint32_t& outImageIndex) {
    Frame& frame = m_frames[m_currentFrame];

    if (frame.timelineValue > 0) {
        uint64_t waitValue = frame.timelineValue;
        vk::SemaphoreWaitInfo waitInfo{};
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_renderTimelineSemaphore;
        waitInfo.pValues = &waitValue;

        vk::Result result = m_device.waitSemaphores(waitInfo, UINT64_MAX);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for timeline semaphore!");
        }

        frame.timelineValue = 0;
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

    if (m_imagesInFlight[outImageIndex] > 0) {
        uint64_t waitValue = m_imagesInFlight[outImageIndex];
        vk::SemaphoreWaitInfo waitInfo{};
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_renderTimelineSemaphore;
        waitInfo.pValues = &waitValue;

        vk::Result result = m_device.waitSemaphores(waitInfo, UINT64_MAX);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for timeline semaphore!");
        }

        m_imagesInFlight[outImageIndex] = 0;
    }

    return true;
}

void FrameManager::endFrame(vk::Queue graphicsQueue, vk::Queue presentQueue, vk::SwapchainKHR swapchain, uint32_t imageIndex) {
    Frame& frame = m_frames[m_currentFrame];

    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    std::array<vk::Semaphore, 2> signalSemaphores{m_renderFinishedSemaphores[imageIndex], m_renderTimelineSemaphore};

    vk::SubmitInfo submitInfo{
        1, &m_imageAvailableSemaphores[m_currentFrame],
        &waitStages,
        1, &frame.commandBuffer,
        static_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
    };

    uint64_t waitValues[] = {0};
    std::array<uint64_t, 2> signalValues{0, ++m_nextTimelineValue};

    vk::TimelineSemaphoreSubmitInfo timelineInfo{};
    timelineInfo.waitSemaphoreValueCount = 1;
    timelineInfo.pWaitSemaphoreValues = waitValues;
    timelineInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalValues.size());
    timelineInfo.pSignalSemaphoreValues = signalValues.data();

    submitInfo.pNext = &timelineInfo;

    graphicsQueue.submit(submitInfo, vk::Fence());

    frame.timelineValue = signalValues.back();
    m_imagesInFlight[imageIndex] = frame.timelineValue;

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
