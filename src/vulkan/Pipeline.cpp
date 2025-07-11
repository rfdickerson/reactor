#include "Pipeline.hpp"
#include <fstream>
#include <stdexcept>

#include "ShaderModule.hpp"
#include "VulkanUtils.hpp"

namespace reactor {

std::vector<char> Pipeline::readFile(const std::string &filename) const {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader file: " + filename);
    size_t            fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

Pipeline::Pipeline(vk::Device device, vk::Format colorAttachmentFormat,
                   const std::string &vertShaderPath, const std::string &fragShaderPath,
                   const std::vector<vk::DescriptorSetLayout> &setLayouts, uint32_t samples,
                   vk::Format depthAttachmentFormat, bool depthWriteEnable)
    : m_device(device) {
    // 1. Read shader code from files
    auto vertShaderCode = readFile(vertShaderPath);
    auto vertShaderModule = ShaderModule(m_device, vertShaderCode);

    vk::PipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.stage  = vk::ShaderStageFlagBits::eVertex;
    vertStageInfo.module = vertShaderModule.getHandle();
    vertStageInfo.pName  = "main";

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {vertStageInfo};

    std::unique_ptr<ShaderModule> fragShaderModule;
    if (!fragShaderPath.empty()) {
        auto fragShaderCode = readFile(fragShaderPath);
        fragShaderModule = std::make_unique<ShaderModule>(m_device, fragShaderCode);

        vk::PipelineShaderStageCreateInfo fragStageInfo{};
        fragStageInfo.stage  = vk::ShaderStageFlagBits::eFragment;
        fragStageInfo.module = fragShaderModule->getHandle();
        fragStageInfo.pName  = "main";
        shaderStages.push_back(fragStageInfo);
    }

    // 3. Vertex input (empty, for a basic triangle with no vertex buffer)
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    // 4. Input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology               = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 5. Viewport and scissor (dynamic for flexibility)
    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    // 6. Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = vk::PolygonMode::eFill;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace               = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable         = VK_FALSE;

    // 7. Multisampling (disabled)
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = utils::mapSampleCountFlag(samples);

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    if (depthAttachmentFormat != vk::Format::eUndefined) {
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = vk::CompareOp::eLess;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
    }

    // 8. Color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = (colorAttachmentFormat != vk::Format::eUndefined) ? 1 : 0;
    colorBlending.pAttachments    = (colorAttachmentFormat != vk::Format::eUndefined) ? &colorBlendAttachment : nullptr;

    // 9. Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = setLayouts.data();

    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

    // 10. Dynamic state (viewport and scissor set in command buffer)
    std::vector<vk::DynamicState>      dynamicStates = {vk::DynamicState::eViewport,
                                                        vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    // 11. Dynamic rendering
    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.colorAttachmentCount    = (colorAttachmentFormat != vk::Format::eUndefined) ? 1 : 0;
    renderingInfo.pColorAttachmentFormats = (colorAttachmentFormat != vk::Format::eUndefined) ? &colorAttachmentFormat : nullptr;
    renderingInfo.viewMask                = 0;
    renderingInfo.depthAttachmentFormat   = depthAttachmentFormat;
    renderingInfo.stencilAttachmentFormat = vk::Format::eUndefined;

    // 11. Create graphics pipeline
    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.layout              = m_pipelineLayout;
    pipelineInfo.renderPass          = VK_NULL_HANDLE;
    pipelineInfo.subpass             = 0;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.pNext               = &renderingInfo;

    auto pipelineResult = m_device.createGraphicsPipeline({}, pipelineInfo);
    if (pipelineResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    m_pipeline = pipelineResult.value;

}

Pipeline::~Pipeline() {
    if (m_pipeline) {
        m_device.destroyPipeline(m_pipeline);
}
    if (m_pipelineLayout) {
        m_device.destroyPipelineLayout(m_pipelineLayout);
}
}

Pipeline::Pipeline(Pipeline&& other) noexcept
    : m_device(other.m_device),
      m_pipelineLayout(other.m_pipelineLayout),
      m_pipeline(other.m_pipeline)
{
    other.m_pipelineLayout = nullptr;
    other.m_pipeline = nullptr;
}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
{
    if (this != &other) {
        // Destroy our own Vulkan objects first
        if (m_pipeline) {
            m_device.destroyPipeline(m_pipeline);
        }
        if (m_pipelineLayout) {
            m_device.destroyPipelineLayout(m_pipelineLayout);
        }
        m_device = other.m_device;
        m_pipelineLayout = other.m_pipelineLayout;
        m_pipeline = other.m_pipeline;

        other.m_pipelineLayout = nullptr;
        other.m_pipeline = nullptr;
    }
    return *this;
}


} // namespace reactor