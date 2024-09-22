#pragma once

#include "Dependencies.hpp"

struct VertexInputLayout
{
    std::vector<VkVertexInputAttributeDescription> m_descs;
    uint32_t m_stride = 0;

    void Add(int location, int size)
    {
        auto format = static_cast<VkFormat>(VK_FORMAT_R32_SFLOAT + (size - 1) * 3);
        m_descs.emplace_back(location, 0, format, m_stride * sizeof(float));
        m_stride += size;
    }
};

class GraphicsPipeline
{
public:

    struct CreateInfo
    {
        VkRenderPass RenderPass;
        char const* Vertex;
        char const* Fragment;
        VertexInputLayout Input;
    };

    GraphicsPipeline(class TestApp const& app, CreateInfo const& info);

    ~GraphicsPipeline();

    [[nodiscard]] VkPipeline GetPipeline() const { return m_pipeline; }

private:

    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipeline_layout;
};


class Framebuffers
{
public:

    explicit Framebuffers(TestApp const& app);

    ~Framebuffers();

    [[nodiscard]] VkRenderPass GetRenderPass() const { return m_render_pass; }
    [[nodiscard]] VkFramebuffer GetFramebuffer(size_t index) const { return m_framebuffers[index]; }
    [[nodiscard]] VkExtent2D GetExtent() const { return m_extent; }
    [[nodiscard]] VkFormat GetFormat() const { return m_format; }

private:

    VkDevice m_device;
    std::vector<VkFramebuffer> m_framebuffers;
    VkRenderPass m_render_pass;

    VkFormat m_format;
    VkExtent2D m_extent;
};