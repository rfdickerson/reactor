#include "ShadowMapping.hpp"

#include "VulkanRenderer.hpp"
#include "../core/Uniforms.hpp"
#include "../logging/Logger.hpp"

namespace reactor
{

ShadowMapping::ShadowMapping(VulkanRenderer& renderer, uint32_t resolution)
    : m_renderer(renderer), m_resolution(resolution), m_shadowMapView(VK_NULL_HANDLE),
      m_shadowMapSampler(VK_NULL_HANDLE)
{
    createResources();
    createDescriptors();
    createPipeline();
}

ShadowMapping::~ShadowMapping()
{
    // if you own any Vulkan handles directly, clean them up here

    auto device = m_renderer.device();
    device.destroyImageView(m_shadowMapView);
    device.destroySampler(m_shadowMapSampler);
}

void ShadowMapping::createResources()
{
    auto device = m_renderer.device();
    auto& allocator = m_renderer.allocator();

    vk::ImageCreateInfo imageInfo = {};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = vk::Format::eD32Sfloat;
    imageInfo.extent = vk::Extent3D{m_resolution, m_resolution, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    m_shadowMap = std::make_unique<Image>(allocator, imageInfo, memoryUsage);

    // 2. Create image view
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = m_shadowMap->get();
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eD32Sfloat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    m_shadowMapView = device.createImageView(viewInfo);

    // 3. Create sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = vk::CompareOp::eLessOrEqual;

    m_shadowMapSampler = device.createSampler(samplerInfo);

    const size_t frameCount = 2;
    m_mvpBuffer.clear();
    m_mvpBuffer.reserve(frameCount);
    for (size_t i = 0; i < frameCount; ++i)
    {
        m_mvpBuffer.push_back(std::make_unique<Buffer>(m_renderer.allocator(),
                                                       sizeof(SceneUBO),
                                                       vk::BufferUsageFlagBits::eUniformBuffer,
                                                       VMA_MEMORY_USAGE_CPU_TO_GPU,
                                                       "MVP Buffer"));
    }
}

void ShadowMapping::createPipeline()
{
    LOG_INFO("Creating shadow mapping pipeline");

    auto device = m_renderer.device();

    std::vector<vk::DescriptorSetLayout> setLayouts;
    if (m_descriptors)
    {
        setLayouts.push_back(m_descriptors->getLayout());
    }

    Pipeline::Builder builder(device);

    builder
        .setVertexShader("../resources/shaders/triangle.vert.spv")
        // No fragment shader, we only want depth output
        .setVertexInputFromVertex()
        .setDepthAttachment(vk::Format::eD32Sfloat, true) // depth test and write enabled
        .enableDepthBias()
        .setDescriptorSetLayouts(setLayouts)
        .setMultisample(1)
        .setCullMode(vk::CullModeFlagBits::eFront)
        .setFrontFace(vk::FrontFace::eClockwise) // Match main geometry pipeline
        .addPushContantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4));

    m_depthPassPipeline = builder.build();
}

void ShadowMapping::createDescriptors()
{
    spdlog::info("Creating shadow mapping descriptors");
    vk::Device device = m_renderer.device();
    size_t framesInFlight = 2;

    // Descriptor set bindings: UBO for light's MVP
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}
        // Add more if you want—for example for a sampler or shadow map
    };

    // Use your DescriptorSet abstraction to handle layout & Vulkan allocation
    m_descriptors = std::make_unique<DescriptorSet>(device, m_renderer.descriptorPool(), framesInFlight, bindings);

    // Now update each descriptor set with the corresponding per-frame UBO
    for (size_t i = 0; i < framesInFlight; ++i)
    {
        vk::DescriptorBufferInfo uboInfo{};
        uboInfo.buffer = m_mvpBuffer[i]->getHandle();
        uboInfo.offset = 0;
        uboInfo.range = sizeof(SceneUBO);

        vk::WriteDescriptorSet writes{};
        writes.dstSet = m_descriptors->get(i);
        writes.dstBinding = 0;
        writes.dstArrayElement = 0;
        writes.descriptorType = vk::DescriptorType::eUniformBuffer;
        writes.descriptorCount = 1;
        writes.pBufferInfo = &uboInfo;

        m_descriptors->updateSet({writes});
    }
}

void ShadowMapping::recordShadowPass(vk::CommandBuffer cmd,
                                     size_t frameIndex,
                                     const std::function<void(vk::CommandBuffer)>& drawCallback)
{
    vk::ClearValue clearDepth;
    clearDepth.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView = m_shadowMapView;
    depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.clearValue = clearDepth;

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea = vk::Rect2D({0, 0}, {m_resolution, m_resolution});
    renderingInfo.layerCount = 1;
    renderingInfo.pDepthAttachment = &depthAttachment;

    cmd.setDepthBias(depthBiasConstant, 0.0f, depthBiasSlope);

    cmd.beginRendering(&renderingInfo);

    // set viewport/scissor
    const vk::Viewport viewport = {0.0f, 0.0f, static_cast<float>(m_resolution), static_cast<float>(m_resolution), 0.0f, 1.0f};
    const vk::Rect2D scissor = {vk::Offset2D{0, 0}, vk::Extent2D{m_resolution, m_resolution}};
    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_depthPassPipeline->get());

    auto descriptor = m_descriptors->get(frameIndex);

    // bind descriptor set (with light MVP)
    cmd.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, m_depthPassPipeline->getLayout(), 0, 1, &descriptor, 0, nullptr);

    // draw the shadow casters
    drawCallback(cmd);

    cmd.endRendering();
}

vk::ImageView ShadowMapping::shadowMapView() const
{
    return m_shadowMapView;
}

vk::Sampler ShadowMapping::shadowMapSampler() const
{
    return m_shadowMapSampler;
}

vk::DescriptorSet ShadowMapping::shadowMapDescriptorSet(size_t frameIndex) const
{
    return m_descriptors->get(frameIndex);
}

vk::Image ShadowMapping::shadowMapImage() const
{
    return m_shadowMap->get();
}

void ShadowMapping::setLightMatrix(const glm::mat4& lightSpaceMatrix, size_t frameIndex)
{
    SceneUBO ubo{};
    ubo.view = glm::mat4(1.0f);
    ubo.projection = lightSpaceMatrix;
    ubo.lightSpaceMatrix = glm::mat4(1.0f);

    // map buffer, copy matrix
    void* data = m_mvpBuffer[frameIndex]->map();
    memcpy(data, &ubo, sizeof(SceneUBO));
    m_mvpBuffer[frameIndex]->unmap();
}

} // namespace reactor