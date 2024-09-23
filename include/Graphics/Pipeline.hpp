#pragma once

#include "Dependencies.hpp"

CLASS_DECLARE(GraphicsDevice);

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
        GraphicsDevice const* Device;
        char const* Vertex;
        char const* Fragment;
        VertexInputLayout Input;
    };

    explicit GraphicsPipeline(CreateInfo const& info);

    ~GraphicsPipeline();

    NODISCARD VkPipeline GetPipeline() const { return m_pipeline; }

private:

    GraphicsDevice const& m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipeline_layout;

};
