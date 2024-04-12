#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>

#include "GP2_Vertex.h"
#include "GP2_DescriptorPool.h"

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

	const VkDescriptorSetLayout& getDescriptorSetLayout() { return m_DescriptorPool->getDescriptorSetLayout(); };

private:
	VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo();
	VkPipelineShaderStageCreateInfo CreateVertexShaderInfo();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	std::string m_VertexShaderFile;
	std::string m_FragmentShaderFile;

	VkVertexInputBindingDescription m_BindingDescription{};
	std::array<VkVertexInputAttributeDescription, 2> m_AttributeDescription{};

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

	VkDevice m_Device{VK_NULL_HANDLE};

	// UBO stuff
	GP2_DescriptorPool<GP2_UBO>* m_DescriptorPool{VK_NULL_HANDLE};
};