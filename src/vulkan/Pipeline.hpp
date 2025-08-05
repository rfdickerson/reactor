#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Vertex.hpp"

namespace reactor
{

    class Pipeline
    {
    public:
        class Builder
        {
        public:
            Builder(vk::Device device);

            Builder& setVertexShader(const std::string& shaderPath);
            Builder& setFragmentShader(const std::string& shaderPath);
            Builder& setColorAttachment(vk::Format format);
            Builder& setDepthAttachment(vk::Format format, bool depthWriteEnable = true);
            Builder& setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts);
            Builder& setVertexInputFromVertex();
            Builder& setMultisample(uint32_t samples);
            Builder& setCullMode(vk::CullModeFlags cullMode);
            Builder& setFrontFace(vk::FrontFace frontFace);
            Builder& addPushContantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size);
            Builder& enableDepthBias(bool enable=true);

            [[nodiscard]] std::unique_ptr<Pipeline> build() const;

        private:
            vk::Device m_device;
            std::string m_vertShaderPath;
            std::string m_fragShaderPath;
            vk::Format m_colorAttachmentFormat = vk::Format::eUndefined;
            vk::Format m_depthAttachmentFormat = vk::Format::eUndefined;
            bool m_depthWriteEnable = true;
            std::vector<vk::DescriptorSetLayout> m_setLayouts;
            uint32_t m_samples = 1;
            vk::CullModeFlags m_cullMode = vk::CullModeFlagBits::eBack;
            vk::FrontFace m_frontFace = vk::FrontFace::eCounterClockwise;
            bool m_depthBiasEnable = false;

            std::vector<vk::VertexInputBindingDescription> m_bindings;
            std::vector<vk::VertexInputAttributeDescription> m_attributes;
            std::vector<vk::PushConstantRange> m_pushRanges;
        };

        ~Pipeline();

        [[nodiscard]] vk::Pipeline get() const
        {
            return m_pipeline;
        }
        [[nodiscard]] vk::PipelineLayout getLayout() const
        {
            return m_pipelineLayout;
        }

        // Delete copy operations
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        // Move operations
        Pipeline(Pipeline&& other) noexcept;
        Pipeline& operator=(Pipeline&& other) noexcept;

    private:
        friend class Builder;
        Pipeline(vk::Device device, vk::PipelineLayout pipelineLayout, vk::Pipeline pipeline);

        vk::Device m_device;
        vk::PipelineLayout m_pipelineLayout;
        vk::Pipeline m_pipeline;
    };

} // namespace reactor