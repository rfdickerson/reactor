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
                 vk::Format depthFormat = vk::Format::eUndefined,
                 const std::vector<vk::VertexInputBindingDescription>& bindings = {},
                 const std::vector<vk::VertexInputAttributeDescription>& attributes = {});
        ~Pipeline();

        vk::Pipeline get() const { return m_pipeline; }
        vk::PipelineLayout getLayout() const { return m_pipelineLayout; }

    private:
        vk::Device m_device;
        vk::PipelineLayout m_pipelineLayout{};
        vk::Pipeline m_pipeline{};

        std::vector<char> readFile(const std::string& filename) const;
        vk::ShaderModule createShaderModule(const std::vector<char>& code);

        // Prevent copying
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
    };

} // namespace reactor