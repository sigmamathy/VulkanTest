#include "Graphics/Pipeline.hpp"

#include "Graphics/Device.hpp"

#define THISFILE "Graphics/Pipeline.cpp"

static VkShaderModule s_CreateShaderModule(VkDevice device, char const* path)
{
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
    ERRCHECK(ifs.is_open());
    size_t fsize = ifs.tellg();
    std::vector<char> buffer(fsize);
    ifs.seekg(0);
    ifs.read(buffer.data(), static_cast<std::streamsize>(fsize));
    ifs.close();

    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.codeSize = buffer.size();
    shader_ci.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule module;
    ERRCHECK(vkCreateShaderModule(device, &shader_ci, nullptr, &module) == VK_SUCCESS);
    return module;
}

GraphicsPipeline::GraphicsPipeline(CreateInfo const& info)
	: m_device(*info.Device), m_pipeline{}, m_pipeline_layout{}
{
	VkDevice device = info.Device->GetDevice();

	VkShaderModule vert = s_CreateShaderModule(device, info.Vertex);
	VkShaderModule frag = s_CreateShaderModule(device, info.Fragment);

	VkPipelineShaderStageCreateInfo shader_stages_ci[] = { {}, {} };

	shader_stages_ci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages_ci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages_ci[0].module = vert;
	shader_stages_ci[0].pName = "main";

	shader_stages_ci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages_ci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages_ci[1].module = frag;
	shader_stages_ci[1].pName = "main";

	VkVertexInputBindingDescription binding_desc{};
	binding_desc.binding = 0;
	binding_desc.stride = info.Input.m_stride * sizeof(float);
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertex_input_ci{};
	vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (info.Input.m_descs.size()) {
		vertex_input_ci.vertexBindingDescriptionCount = 1;
		vertex_input_ci.vertexAttributeDescriptionCount = info.Input.m_descs.size();
		vertex_input_ci.pVertexBindingDescriptions = &binding_desc;
		vertex_input_ci.pVertexAttributeDescriptions = info.Input.m_descs.data();
	}

	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci{};
	input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_ci.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_ci{};
	viewport_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_ci.viewportCount = 1;
	viewport_ci.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer_ci{};
	rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_ci.depthClampEnable = VK_FALSE;
	rasterizer_ci.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_ci.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_ci.lineWidth = 1.0f;
	rasterizer_ci.cullMode = VK_CULL_MODE_NONE;
	rasterizer_ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer_ci.depthBiasEnable = VK_FALSE;
	rasterizer_ci.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer_ci.depthBiasClamp = 0.0f; // Optional
	rasterizer_ci.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_ci{};
	multisampling_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_ci.sampleShadingEnable = VK_FALSE;
	multisampling_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling_ci.minSampleShading = 1.0f; // Optional
	multisampling_ci.pSampleMask = nullptr; // Optional
	multisampling_ci.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling_ci.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState blend_func{};
	blend_func.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend_func.blendEnable = VK_TRUE;
	blend_func.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blend_func.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_func.colorBlendOp = VK_BLEND_OP_ADD;
	blend_func.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_func.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_func.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blending_ci{};
	blending_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending_ci.logicOpEnable = VK_FALSE;
	blending_ci.logicOp = VK_LOGIC_OP_COPY; // Optional
	blending_ci.attachmentCount = 1;
	blending_ci.pAttachments = &blend_func;
	blending_ci.blendConstants[0] = 0.0f; // Optional
	blending_ci.blendConstants[1] = 0.0f; // Optional
	blending_ci.blendConstants[2] = 0.0f; // Optional
	blending_ci.blendConstants[3] = 0.0f; // Optional

	constexpr std::array dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
	dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_ci.dynamicStateCount = dynamic_states.size();
	dynamic_state_ci.pDynamicStates = dynamic_states.data();

	VkPipelineLayoutCreateInfo pipeline_layout_ci{};
	pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ERRCHECK(vkCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &m_pipeline_layout) == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_ci.stageCount = 2;
	pipeline_ci.pStages = shader_stages_ci;
	pipeline_ci.pVertexInputState = &vertex_input_ci;
	pipeline_ci.pInputAssemblyState = &input_assembly_ci;
	pipeline_ci.pViewportState = &viewport_ci;
	pipeline_ci.pRasterizationState = &rasterizer_ci;
	pipeline_ci.pMultisampleState = &multisampling_ci;
	pipeline_ci.pColorBlendState = &blending_ci;
	pipeline_ci.pDynamicState = &dynamic_state_ci;
	pipeline_ci.layout = m_pipeline_layout;
	pipeline_ci.renderPass = info.Device->GetRenderPass();
	pipeline_ci.subpass = 0;
	pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;

	ERRCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &m_pipeline) == VK_SUCCESS);

	vkDestroyShaderModule(device, vert, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device.GetDevice(), m_pipeline_layout, nullptr);
}