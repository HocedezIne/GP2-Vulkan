#pragma once

#include <vulkanbase/VulkanUtil.h>
#include <string>

#include "CommandBuffer.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_DescriptorPool.h"
#include "GP2_ImageBuffer.h"

enum class GP2_PBRRenderModes {
	Combined,
	Albedo,
	Normal,
	Specular
};

template <class UBO, class Vertex>
class GP2_PBRBasePipeline
{
public:
	GP2_PBRBasePipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~GP2_PBRBasePipeline() = default;

	virtual void Initialize(const VulkanContext& context);
	virtual void CleanUp();

	void Record(const GP2_CommandBuffer& cmdBuffer, VkExtent2D extent, int imageIndex);

	void AddMesh(std::unique_ptr<GP2_Mesh<Vertex>> mesh);

	void SetUBO(UBO ubo, size_t uboIndex);

	void CycleRenderMode() { m_RenderMode = static_cast<GP2_PBRRenderModes>((int(m_RenderMode) + 1) % 4); };

protected:
	GP2_DescriptorPool<UBO>* m_DescriptorPool{ nullptr };

private: 
	void DrawScene(const GP2_CommandBuffer& cmdBuffer);
	void CreateGraphicsPipeline();

	static std::vector<VkPushConstantRange> CreatePushConstantRange();

	VkDevice m_Device{ VK_NULL_HANDLE };

	VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	GP2_Shader<Vertex> m_Shader;

	std::vector<std::unique_ptr<GP2_Mesh<Vertex>>> m_Meshes{};

	GP2_PBRRenderModes m_RenderMode{ GP2_PBRRenderModes::Combined };
};

template <class UBO, class Vertex>
GP2_PBRBasePipeline<UBO, Vertex>::GP2_PBRBasePipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_Shader{ vertexShaderFile, fragmentShaderFile }
{ }

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::AddMesh(std::unique_ptr<GP2_Mesh<Vertex>> mesh)
{
	m_Meshes.push_back(std::move(mesh));
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::CleanUp()
{
	for (auto& mesh : m_Meshes)
	{
		mesh->DestroyMesh();
	}

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

	delete m_DescriptorPool;
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::Initialize(const VulkanContext& context)
{
	m_Device = context.device;
	m_RenderPass = context.renderPass;

	m_Shader.Initialize(context.device);

	CreateGraphicsPipeline();
}

template <class UBO, class Vertex>
std::vector<VkPushConstantRange> GP2_PBRBasePipeline<UBO, Vertex>::CreatePushConstantRange()
{
	std::vector<VkPushConstantRange> pushConstantRanges(2);
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = sizeof(GP2_MeshData);

	pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[1].offset = sizeof(GP2_MeshData);
	pushConstantRanges[1].size = sizeof(m_RenderMode);

	return pushConstantRanges;
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::CreateGraphicsPipeline()
{
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorPool->GetDescriptorSetLayout();
	pipelineLayoutInfo.pushConstantRangeCount = 2;
	auto pushConstantRanges = CreatePushConstantRange();
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.f;
	depthStencil.maxDepthBounds = 1.f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkGraphicsPipelineCreateInfo pipelineInfo{};

	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = m_Shader.GetShaderStages().data();
	pipelineInfo.pVertexInputState = &m_Shader.CreateVertexInputStateInfo();
	pipelineInfo.pInputAssemblyState = &m_Shader.CreateInputAssemblyStateInfo();
	pipelineInfo.pDepthStencilState = &depthStencil;

#pragma region pipelineInfo
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
#pragma endregion pipelineInfo


	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	m_Shader.DestroyShaderModules();
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::DrawScene(const GP2_CommandBuffer& cmdBuffer)
{
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(m_PipelineLayout, cmdBuffer.GetVkCommandBuffer());
	}
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::Record(const GP2_CommandBuffer& cmdBuffer, VkExtent2D extent, int imageIndex)
{
	vkCmdBindPipeline(cmdBuffer.GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmdBuffer.GetVkCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(cmdBuffer.GetVkCommandBuffer(), 0, 1, &scissor);

	m_DescriptorPool->BindDescriptorSet(cmdBuffer.GetVkCommandBuffer(), m_PipelineLayout, imageIndex);

	vkCmdPushConstants(cmdBuffer.GetVkCommandBuffer(), m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GP2_MeshData), sizeof(m_RenderMode), &m_RenderMode);

	DrawScene(cmdBuffer);
}

template <class UBO, class Vertex>
void GP2_PBRBasePipeline<UBO, Vertex>::SetUBO(UBO ubo, size_t uboIndex)
{
	m_DescriptorPool->SetUBO(ubo, uboIndex);
}