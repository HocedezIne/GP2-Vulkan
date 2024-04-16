#pragma once
#include "vulkan/vulkan_core.h"

#include <vulkanbase/VulkanUtil.h>

#include "CommandBuffer.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"

class GP2_GraphicsPipeline2D
{
public:
	GP2_GraphicsPipeline2D(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_GraphicsPipeline2D() = default;

	void Initialize(const VulkanContext& context);

	VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyInfo();

	void CleanUp();

	void Record(const GP2_CommandBuffer& cmdBuffer, VkExtent2D extent);
	void DrawScene(const GP2_CommandBuffer& cmdBuffer);

	void AddMesh(std::unique_ptr<GP2_Mesh> mesh);

	void SetUBO(GP2_ViewProjection ubo, size_t uboIndex);

private:
	void CreateGraphicsPipeline();

	VkPushConstantRange CreatePushConstantRange();

	VkDevice m_Device{ VK_NULL_HANDLE };

	VkPipeline m_GraphicsPipeline{VK_NULL_HANDLE};
	VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	GP2_Shader m_Shader;

	std::vector<std::unique_ptr<GP2_Mesh>> m_Meshes;
};