//
// Created by rfdic on 6/16/2025.
//

#include "Imgui.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace reactor {

    Imgui::Imgui(VulkanContext &vulkanContext, Window &window)
        : m_device(vulkanContext.device())
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // --- Initialize descriptor pool for ImGui ---
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        // create the pool
        m_descriptorPool = vulkanContext.device().createDescriptorPool(pool_info);

        // GLFW backend init
        ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);

        auto colorAttachmentFormat = vk::Format::eB8G8R8A8Srgb;

        vk::PipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
        renderingInfo.viewMask = 0;
        renderingInfo.depthAttachmentFormat = vk::Format::eUndefined;
        renderingInfo.stencilAttachmentFormat = vk::Format::eUndefined;

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = vulkanContext.instance();
        initInfo.PhysicalDevice = vulkanContext.physicalDevice();
        initInfo.Device = vulkanContext.device();
        initInfo.QueueFamily = vulkanContext.queueFamilies().graphicsFamily.value();
        initInfo.Queue = vulkanContext.graphicsQueue();
        initInfo.PipelineCache = nullptr;
        initInfo.DescriptorPool = m_descriptorPool;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineRenderingCreateInfo = renderingInfo;

        ImGui_ImplVulkan_Init(&initInfo);
    }


    Imgui::~Imgui() {

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
        m_device.destroyDescriptorPool(m_descriptorPool);
    }

void Imgui::createFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
        ImGui::Begin("Composite");
        ImGui::SliderFloat("Exposure", &m_exposure, 0.0, 1.0);
        ImGui::SliderFloat("Contrast", &m_contrast, 0.0, 1.0);
        ImGui::SliderFloat("Saturation", &m_saturation, 0.0, 1.0);

        ImGui::End();
}

    void Imgui::drawFrame(vk::CommandBuffer commandBuffer) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }



} // reactor