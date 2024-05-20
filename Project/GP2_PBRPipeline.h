#pragma once

#include <vulkanbase/VulkanUtil.h>

#include "CommandBuffer.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_DescriptorPool.h"
#include "GP2_ImageBuffer.h"

template <class UBO, class Vertex>
class GP2_PBRPipeline
{
public:
	GP2_PBRPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_PBRPipeline() = default;

	void SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue);

	void Initialize(const VulkanContext& context, size_t descriptorPoolCount);

	void CleanUp();

	void Record(const GP2_CommandBuffer& cmdBuffer, VkExtent2D extent, int imageIndex);
	void DrawScene(const GP2_CommandBuffer& cmdBuffer);

	void AddMesh(std::unique_ptr<GP2_Mesh<Vertex>> mesh);

	void SetUBO(UBO ubo, size_t uboIndex);

private:
	void CreateGraphicsPipeline();

	VkPushConstantRange CreatePushConstantRange();

	VkDevice m_Device{ VK_NULL_HANDLE };

	VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	GP2_Shader<Vertex> m_Shader;

	GP2_ImageBuffer* m_DiffuseMap;
	GP2_ImageBuffer* m_NormalMap;

	GP2_DescriptorPool<UBO>* m_DescriptorPool{};

	std::vector<std::unique_ptr<GP2_Mesh<Vertex>>> m_Meshes{};
};

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::AddMesh(std::unique_ptr<GP2_Mesh<Vertex>> mesh)
{
	m_Meshes.push_back(std::move(mesh));
}

template<class UBO, class Vertex>
inline void GP2_PBRPipeline<UBO, Vertex>::SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_DiffuseMap = new GP2_ImageBuffer{ context };
	m_DiffuseMap->LoadImageData(diffuse, context);
	m_DiffuseMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	m_NormalMap = new GP2_ImageBuffer{ context };
	m_NormalMap->LoadImageData(normal, context);
	m_NormalMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::CleanUp()
{
	for (auto& mesh : m_Meshes)
	{
		mesh->DestroyMesh();
	}

	m_DiffuseMap->Destroy();
	m_DiffuseMap = nullptr;
	m_NormalMap->Destroy();
	m_NormalMap = nullptr;

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

	delete m_DescriptorPool;
}

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::Initialize(const VulkanContext& context, size_t descriptorPoolCount)
{
	m_Device = context.device;
	m_RenderPass = context.renderPass;

	m_Shader.Initialize(context.device);

	std::vector<std::pair<VkImageView, VkSampler>> imageDatas;
	imageDatas.push_back(std::make_pair(m_DiffuseMap->GetView(), m_DiffuseMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_NormalMap->GetView(), m_NormalMap->GetSampler()));
	m_DescriptorPool = new GP2_DescriptorPool<UBO>{ context.device, descriptorPoolCount };
	m_DescriptorPool->Initialize(context, imageDatas.size());
	m_DescriptorPool->CreateDescriptorSets(imageDatas);

	CreateGraphicsPipeline();
}

template <class UBO, class Vertex>
GP2_PBRPipeline<UBO, Vertex>::GP2_PBRPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_Shader{ vertexShaderFile, fragmentShaderFile }
{

}

template <class UBO, class Vertex>
VkPushConstantRange GP2_PBRPipeline<UBO, Vertex>::CreatePushConstantRange()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(GP2_MeshData);

	return pushConstantRange;
}

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::CreateGraphicsPipeline()
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
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	VkPushConstantRange pushConstantRange = CreatePushConstantRange();
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
void GP2_PBRPipeline<UBO, Vertex>::DrawScene(const GP2_CommandBuffer& cmdBuffer)
{
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(m_PipelineLayout, cmdBuffer.GetVkCommandBuffer());
	}
}

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::Record(const GP2_CommandBuffer& cmdBuffer, VkExtent2D extent, int imageIndex)
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

	DrawScene(cmdBuffer);
}

template <class UBO, class Vertex>
void GP2_PBRPipeline<UBO, Vertex>::SetUBO(UBO ubo, size_t uboIndex)
{
	m_DescriptorPool->SetUBO(ubo, uboIndex);
}