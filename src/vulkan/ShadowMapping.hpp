#pragma once

#include "Buffer.hpp"
#include "DescriptorSet.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace reactor
{

class VulkanRenderer;

class ShadowMapping
{
public:
    explicit ShadowMapping(VulkanRenderer& renderer, uint32_t resolution = 2048);
    ~ShadowMapping();

    void recordShadowPass(vk::CommandBuffer cmd, size_t frameIndex, const std::function<void(vk::CommandBuffer)>& drawCallback);

    vk::ImageView shadowMapView() const;
    vk::Sampler shadowMapSampler() const;
    vk::DescriptorSet shadowMapDescriptorSet(size_t frameIndex) const;

    void setLightMatrix(const glm::mat4& lightMVP, size_t frameIndex);

    uint32_t resolution() const
    {
        return m_resolution;
    }

private:
    void createResources();
    void createPipeline();
    void createDescriptors();

    const float depthBiasConstant = 1.25f;
    const float depthBiasSlope = 1.75f;

    VulkanRenderer& m_renderer;
    uint32_t m_resolution;

    // depth image, view, and sampler for shadow map
    std::unique_ptr<Image> m_shadowMap;
    vk::ImageView m_shadowMapView;
    vk::Sampler m_shadowMapSampler;

    std::vector<std::unique_ptr<Buffer>> m_mvpBuffer;

    std::unique_ptr<DescriptorSet> m_descriptors;

    // Pipeline for depth pass
    std::unique_ptr<Pipeline> m_depthPassPipeline;
};
} // namespace reactor