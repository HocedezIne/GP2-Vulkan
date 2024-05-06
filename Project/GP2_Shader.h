#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>

#include "GP2_Vertex.h"

template<class Vertex>
class GP2_Shader final
{
public:
	GP2_Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
		m_VertexShaderFile(vertexShaderFile), m_FragmentShaderFile(fragmentShaderFile)
	{};

	void Initialize(const VkDevice& vkDevice);
	~GP2_Shader() { DestroyShaderModules(); };

	void DestroyShaderModules();

	GP2_Shader(const GP2_Shader&) = delete;
	GP2_Shader& operator=(const GP2_Shader&) = delete;
	GP2_Shader(const GP2_Shader&&) = delete;
	GP2_Shader& operator=(const GP2_Shader&&) = delete;

	VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo();

	std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; };

private:
	VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo();
	VkPipelineShaderStageCreateInfo CreateVertexShaderInfo();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	std::string m_VertexShaderFile;
	std::string m_FragmentShaderFile;

	VkVertexInputBindingDescription m_BindingDescription{};
	VkVertexInputAttributeDescription* m_AttributeDescriptions{};
	uint32_t m_AttributeCount{};

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

	VkDevice m_Device{VK_NULL_HANDLE};
};

template<class Vertex>
void GP2_Shader<Vertex>::Initialize(const VkDevice& vkDevice)
{
	m_Device = vkDevice;

	m_BindingDescription = Vertex::GetBindingDescription();

	auto attriDescriptions = Vertex::GetAttributeDescriptions();
	m_AttributeCount = static_cast<uint32_t>(attriDescriptions.size());
	m_AttributeDescriptions = new VkVertexInputAttributeDescription[m_AttributeCount];
	std::copy(attriDescriptions.begin(), attriDescriptions.end(), m_AttributeDescriptions);

	m_ShaderStages.push_back(CreateVertexShaderInfo());
	m_ShaderStages.push_back(CreateFragmentShaderInfo());
}

template<class Vertex>
void GP2_Shader<Vertex>::DestroyShaderModules()
{
	for (auto& stageInfo : m_ShaderStages)
	{
		vkDestroyShaderModule(m_Device, stageInfo.module, nullptr);
	}

	m_ShaderStages.clear();
}

template<class Vertex>
VkPipelineVertexInputStateCreateInfo GP2_Shader<Vertex>::CreateVertexInputStateInfo()
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = m_AttributeCount;
	vertexInputInfo.pVertexBindingDescriptions = &m_BindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions;
	return vertexInputInfo;
}

template<class Vertex>
VkPipelineInputAssemblyStateCreateInfo GP2_Shader<Vertex>::CreateInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

template<class Vertex>
VkPipelineShaderStageCreateInfo GP2_Shader<Vertex>::CreateFragmentShaderInfo()
{
	std::vector<char> fragShaderCode = readFile(m_FragmentShaderFile); //"shaders/shader.frag.spv"
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	return fragShaderStageInfo;
}

template<class Vertex>
VkPipelineShaderStageCreateInfo GP2_Shader<Vertex>::CreateVertexShaderInfo()
{
	std::vector<char> vertShaderCode = readFile(m_VertexShaderFile); //"shaders/shader.vert.spv"
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	return vertShaderStageInfo;
}

template<class Vertex>
VkShaderModule GP2_Shader<Vertex>::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}