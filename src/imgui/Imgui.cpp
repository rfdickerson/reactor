//
// Created by rfdic on 6/16/2025.
//

#include "Imgui.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace reactor {

Imgui::Imgui(VulkanContext &vulkanContext, Window &window) : m_device(vulkanContext.device()) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    // --- Initialize descriptor pool for ImGui ---
    VkDescriptorPoolSize       pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                               {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                               {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                               {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo pool_info    = {};
    pool_info.sType                         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                       = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount                 = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes                    = pool_sizes;

    // create the pool
    m_descriptorPool = vulkanContext.device().createDescriptorPool(pool_info);

    // GLFW backend init
    ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);

    auto colorAttachmentFormat = vk::Format::eB8G8R8A8Srgb;

    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.colorAttachmentCount    = 1;
    renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
    renderingInfo.viewMask                = 0;
    renderingInfo.depthAttachmentFormat   = vk::Format::eUndefined;
    renderingInfo.stencilAttachmentFormat = vk::Format::eUndefined;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance                    = vulkanContext.instance();
    initInfo.PhysicalDevice              = vulkanContext.physicalDevice();
    initInfo.Device                      = vulkanContext.device();
    initInfo.QueueFamily                 = vulkanContext.queueFamilies().graphicsFamily.value();
    initInfo.Queue                       = vulkanContext.graphicsQueue();
    initInfo.PipelineCache               = nullptr;
    initInfo.DescriptorPool              = m_descriptorPool;
    initInfo.MinImageCount               = 3;
    initInfo.ImageCount                  = 3;
    initInfo.UseDynamicRendering         = true;
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

    ShowDockspace();
    ShowSceneView();
    ShowInspector();
    ShowConsole();
}

void Imgui::drawFrame(vk::CommandBuffer commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void Imgui::ShowDockspace() {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags     window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport *viewport     = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // Create dockspace ID
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    static bool first_time = true;
    if (first_time) {
        SetupInitialDockLayout(dockspace_id);
        first_time = false;
    }

    ImGui::End();
}

void Imgui::ShowSceneView() {
    ImGui::Begin("Scene View");
    const ImVec2 size = ImGui::GetContentRegionAvail();

    if (m_sceneImguiId) {
        const ImTextureID id =
            reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(m_sceneImguiId));

        ImGui::Image(
            id,
            size,
            ImVec2(0, 1),
            ImVec2(1, 0));
    } else {
        ImGui::Text("No scene image");
    }

    ImGui::End();
}

void Imgui::ShowInspector() {
    ImGui::Begin("Inspector");
    ImGui::SliderFloat("Exposure", &m_exposure, 0.0, 2.0);
    ImGui::SliderFloat("Contrast", &m_contrast, 0.0, 2.0);
    ImGui::SliderFloat("Saturation", &m_saturation, 0.0, 2.0);
    ImGui::End();
}

void Imgui::ShowConsole() {
    ImGui::Begin("Console");
    ImGui::Text("Console");
    ImGui::End();
}

void Imgui::SetupInitialDockLayout(ImGuiID dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear existing layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split main dockspace into left (main), right (inspector), and bottom (console)
    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    // Assign windows to dock nodes
    ImGui::DockBuilderDockWindow("Scene View", dock_main);
    ImGui::DockBuilderDockWindow("Inspector", dock_right);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);
    ImGui::DockBuilderDockWindow("Composite", dock_main);  // or move to its own panel

    ImGui::DockBuilderFinish(dockspace_id);
}


vk::DescriptorSet Imgui::createDescriptorSet(vk::ImageView imageView, vk::Sampler sampler) {

    // if (m_sceneImguiId) {
    //     ImGui_ImplVulkan_RemoveTexture(m_sceneImguiId);
    // }

    return ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

} // namespace reactor