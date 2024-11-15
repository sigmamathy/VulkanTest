#pragma once

#include "Dependencies.hpp"
#include <vulkan/vulkan_core.h>

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

struct DescriptorSetLayout 
{
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	uint32_t m_uniform_buffer_count = 0;

	void AddUniformBuffer(int bind, VkShaderStageFlags stage)
	{
		m_bindings.emplace_back(bind, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, stage, nullptr);
		++m_uniform_buffer_count;
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
		DescriptorSetLayout Descriptors[4];
		uint32_t DescriptorSetsMultiplier = 1;
    };

    explicit GraphicsPipeline(CreateInfo const& info);

    ~GraphicsPipeline();

    NODISCARD VkPipeline GetPipeline() const { return m_pipeline; }

	void WriteDescriptor(int sid, int rid, VkBuffer buffer, size_t size);

private:

    GraphicsDevice const& m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipeline_layout;

	std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
	VkDescriptorPool m_descriptor_pool;
	std::vector<VkDescriptorSet> m_descriptor_sets;
};
