#include "Pipeline.hpp"
#include <fstream>
#include <stdexcept>

namespace reactor {

std::vector<char> Pipeline::readFile(const std::string& filename) const {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader file: " + filename);
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

vk::ShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    return m_device.createShaderModule(createInfo);
}

Pipeline::Pipeline(vk::Device device, vk::Format colorAttachmentFormat,
                   const std::string& vertShaderPath, const std::string& fragShaderPath)
    : m_device(device)
{
    // 1. Read shader code from files
    auto vertShaderCode = readFile(vertShaderPath);
    auto fragShaderCode = readFile(fragShaderPath);

    // 2. Create shader modules
    vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

    // 3. Vertex input (empty, for a basic triangle with no vertex buffer)
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    // 4. Input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 5. Viewport and scissor (dynamic for flexibility)
    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // 6. Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    // 7. Multisampling (disabled)
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    // 8. Color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                          vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB |
                                          vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 9. Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

    // 10. Dynamic state (viewport and scissor set in command buffer)
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // 11. Dynamic rendering
    vk::PipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
    renderingInfo.viewMask = 0;
    renderingInfo.depthAttachmentFormat = vk::Format::eUndefined;
    renderingInfo.stencilAttachmentFormat = vk::Format::eUndefined;

    // 11. Create graphics pipeline
    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pNext = &renderingInfo;

    auto pipelineResult = m_device.createGraphicsPipeline({}, pipelineInfo);
    if (pipelineResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    m_pipeline = pipelineResult.value;

    // Cleanup shader modules
    m_device.destroyShaderModule(vertShaderModule);
    m_device.destroyShaderModule(fragShaderModule);
}

Pipeline::~Pipeline() {
    if (m_pipeline) m_device.destroyPipeline(m_pipeline);
    if (m_pipelineLayout) m_device.destroyPipelineLayout(m_pipelineLayout);
}

} // namespace reactor