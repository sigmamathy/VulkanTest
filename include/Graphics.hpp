#pragma once

#include "Dependencies.hpp"

class GraphicsPipeline
{
public:

    GraphicsPipeline(class TestApp const& app, class Framebuffers const& framebuffers, std::string const& vert_path, std::string const& frag_path);

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