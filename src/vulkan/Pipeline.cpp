#include "Pipeline.hpp"
#include <fstream>
#include <stdexcept>

#include "ShaderModule.hpp"
#include "VulkanUtils.hpp"

namespace reactor
{

    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open shader file: " + filename);
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        return buffer;
    }

    Pipeline::Builder::Builder(vk::Device device)
        : m_device(device)
    {}

    Pipeline::Builder& Pipeline::Builder::setVertexShader(const std::string& shaderPath)
    {
        m_vertShaderPath = shaderPath;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setFragmentShader(const std::string& shaderPath)
    {
        m_fragShaderPath = shaderPath;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setColorAttachment(vk::Format format)
    {
        m_colorAttachmentFormat = format;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setDepthAttachment(vk::Format format, bool depthWriteEnable)
    {
        m_depthAttachmentFormat = format;
        m_depthWriteEnable = depthWriteEnable;
        return *this;
    }

    Pipeline::Builder&
    Pipeline::Builder::setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts)
    {
        m_setLayouts = layouts;
        return *this;
    }
    Pipeline::Builder& Pipeline::Builder::setVertexInputFromVertex()
    {
        m_bindings.push_back(Vertex::getBindingDescription());
        auto attrs = Vertex::getAttributeDescriptions();
        m_attributes.insert(m_attributes.end(), attrs.begin(), attrs.end());
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setMultisample(uint32_t samples)
    {
        m_samples = samples;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setCullMode(vk::CullModeFlags cullMode)
    {
        m_cullMode = cullMode;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::setFrontFace(vk::FrontFace frontFace)
    {
        m_frontFace = frontFace;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::addPushContantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size)
    {
        m_pushRanges.push_back({stages, offset, size});
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::enableDepthBias(bool enable)
    {
        m_depthBiasEnable = enable;
        return *this;
    }


    std::unique_ptr<Pipeline> Pipeline::Builder::build() const
    {
        // 1. Shader Stages
        auto vertShaderCode = readFile(m_vertShaderPath);
        auto vertShaderModule = ShaderModule(m_device, vertShaderCode);
        vk::PipelineShaderStageCreateInfo vertStageInfo({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule.getHandle(), "main");

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {vertStageInfo};
        std::unique_ptr<ShaderModule> fragShaderModule;
        if (!m_fragShaderPath.empty())
        {
            auto fragShaderCode = readFile(m_fragShaderPath);
            fragShaderModule = std::make_unique<ShaderModule>(m_device, fragShaderCode);
            vk::PipelineShaderStageCreateInfo fragStageInfo({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule->getHandle(), "main");
            shaderStages.push_back(fragStageInfo);
        }

        // 2. Vertex Input
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size());
        vertexInputInfo.pVertexBindingDescriptions = m_bindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = m_attributes.data();

        // 3. Input Assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

        // 4. Viewport and Scissor (Dynamic)
        vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

        // 5. Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer(
            {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, m_cullMode, m_frontFace, m_depthBiasEnable? VK_TRUE : VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);

        // 6. Multisampling
        vk::PipelineMultisampleStateCreateInfo multisampling({}, utils::mapSampleCountFlag(m_samples), VK_FALSE);

        // 7. Depth/Stencil
        vk::PipelineDepthStencilStateCreateInfo depthStencil{};
        if (m_depthAttachmentFormat != vk::Format::eUndefined)
        {
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = m_depthWriteEnable ? VK_TRUE : VK_FALSE;
            depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
        }

        // 8. Color Blending
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = (m_colorAttachmentFormat != vk::Format::eUndefined) ? 1 : 0;
        colorBlending.pAttachments =
            (m_colorAttachmentFormat != vk::Format::eUndefined) ? &colorBlendAttachment : nullptr;

        // 9. Dynamic State
        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                       vk::DynamicState::eScissor};
        // conditionally add eDepthBias to the list of dynamic states
        if (m_depthBiasEnable)
        {
            dynamicStates.push_back(vk::DynamicState::eDepthBias);
        }

        vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

        // 10. Pipeline Layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, m_setLayouts);
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_pushRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = m_pushRanges.data();
        vk::PipelineLayout pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

        // 11. Dynamic Rendering Info
        vk::PipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.viewMask = 0;
        renderingInfo.colorAttachmentCount =
            (m_colorAttachmentFormat != vk::Format::eUndefined) ? 1 : 0;
        renderingInfo.pColorAttachmentFormats = &m_colorAttachmentFormat;
        renderingInfo.depthAttachmentFormat = m_depthAttachmentFormat;

        // 12. Graphics Pipeline Create Info
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState =
            (m_depthAttachmentFormat != vk::Format::eUndefined) ? &depthStencil : nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;

        auto result = m_device.createGraphicsPipeline({}, pipelineInfo);
        if (result.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        // Using a private constructor to pass ownership of the created handles
        return std::unique_ptr<Pipeline>(new Pipeline(m_device, pipelineLayout, result.value));
    }

    // --- Pipeline Implementation ---

    Pipeline::Pipeline(vk::Device device, vk::PipelineLayout layout, vk::Pipeline pipeline)
        : m_device(device), m_pipelineLayout(layout), m_pipeline(pipeline)
    {}

    Pipeline::~Pipeline()
    {
        if (m_pipeline)
        {
            m_device.destroyPipeline(m_pipeline);
        }
        if (m_pipelineLayout)
        {
            m_device.destroyPipelineLayout(m_pipelineLayout);
        }
    }

    Pipeline::Pipeline(Pipeline&& other) noexcept
        : m_device(other.m_device), m_pipelineLayout(other.m_pipelineLayout),
          m_pipeline(other.m_pipeline)
    {
        other.m_pipelineLayout = nullptr;
        other.m_pipeline = nullptr;
    }

    Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        if (this != &other)
        {
            if (m_pipeline)
                m_device.destroyPipeline(m_pipeline);
            if (m_pipelineLayout)
                m_device.destroyPipelineLayout(m_pipelineLayout);

            m_device = other.m_device;
            m_pipelineLayout = other.m_pipelineLayout;
            m_pipeline = other.m_pipeline;

            other.m_pipelineLayout = nullptr;
            other.m_pipeline = nullptr;
        }
        return *this;
    }

} // namespace reactor