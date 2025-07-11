#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace reactor {

    class Pipeline {
    public:
        Pipeline(vk::Device device, vk::Format colorAttachmentFormat,
                 const std::string& vertShaderPath, const std::string& fragShaderPath,
                 const std::vector<vk::DescriptorSetLayout>& setLayouts, uint32_t samples,
                 vk::Format depthAttachmentFormat = vk::Format::eUndefined, bool depthWriteEnable = true);
        ~Pipeline();

        [[nodiscard]] vk::Pipeline get() const { return m_pipeline; }
        [[nodiscard]] vk::PipelineLayout getLayout() const { return m_pipelineLayout; }

        // Delete copy operations
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        // Move operations
        Pipeline(Pipeline&& other) noexcept;
        Pipeline& operator=(Pipeline&& other) noexcept;

    private:
        vk::Device m_device;
        vk::PipelineLayout m_pipelineLayout;
        vk::Pipeline m_pipeline;

        [[nodiscard]] std::vector<char> readFile(const std::string& filename) const;


    };

} // namespace reactor