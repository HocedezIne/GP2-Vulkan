#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>

#include "GP2_UniformmBufferObject.h"
#include "GP2_Buffer.h"

class GP2_DescriptorPool
{
public:
	GP2_DescriptorPool(VkDevice device, size_t count);
	~GP2_DescriptorPool();

	void Initialize(const VulkanContext& context);

	void SetUBO(UniformBufferObject data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; };

	void CreateDescriptorSets();

	void BindDescriptorSet(VkCommandBuffer cmdBuffer, VkPipelineLayout layout, size_t index);

private:
	VkDevice m_Device;
	VkDeviceSize m_Size;
	VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};

	void CreateDescriptorSetLayout();
	void CreateUBOs(const VulkanContext& context);

	VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
	std::vector<VkDescriptorSet> m_DescriptorSets;

	std::vector<GP2_Buffer*> m_UBOs;
	std::vector<void*> m_UBOsMapped;

	size_t m_Count;
};