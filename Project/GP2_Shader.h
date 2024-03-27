#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>

#include "GP2_Vertex.h"

const std::vector<GP2_Vertex> m_Vertices = {
{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

class GP2_Shader final
{
public:
	GP2_Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
		m_VertexShaderFile(vertexShaderFile), m_FragmentShaderFile(fragmentShaderFile)
	{};

	void Initialize(const VkDevice& vkDevice);
	~GP2_Shader() = default;

	void DestroyShaderModules(const VkDevice& vkdevice);

	GP2_Shader(const GP2_Shader&) = delete;
	GP2_Shader& operator=(const GP2_Shader&) = delete;
	GP2_Shader(const GP2_Shader&&) = delete;
	GP2_Shader& operator=(const GP2_Shader&&) = delete;

	VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo();

	std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; };

private:
	VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo(const VkDevice& vkDevice);
	VkPipelineShaderStageCreateInfo CreateVertexShaderInfo(const VkDevice& vkDevice);

	VkShaderModule CreateShaderModule(const VkDevice& vkDevice, const std::vector<char>& code);

	std::string m_VertexShaderFile;
	std::string m_FragmentShaderFile;

	VkVertexInputBindingDescription m_BindingDescription{};
	std::array<VkVertexInputAttributeDescription, 2> m_AttributeDescription{};

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
};