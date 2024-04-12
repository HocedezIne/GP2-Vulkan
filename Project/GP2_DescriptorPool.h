#pragma once
#include <vulkan/vulkan_core.h>

#include "CommandBuffer.h"
#include "GP2_UniformmBufferObject.h"

template<class UBO>
class GP2_DescriptorPool
{
public:
	GP2_DescriptorPool(VkDevice device, size_t count);
	~GP2_DescriptorPool();

	void Initialize();

	void setUBO(UBO data, size_t index);

	const VkDescriptorSetLayout& getDescriptorSetLayout() { return m_DescriptorSetLayout; };

	void CreateDescriptorSets();
	void BindDescriptorSet(GP2_CommandBuffer cmdBuffer, VkPipelineLayout layout, size_t index);

private:
	VkDevice m_Device;
	VkDeviceSize m_Size;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	void CreateDescriptorSetLayout();
	void CreateUBOs();

	VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
	std::vector<VkDescriptorSet> m_DescriptorSets{VK_NULL_HANDLE};
	std::vector<GP2_UniformBufferObjectPtr<UBO>>m_UBOs;

	size_t m_Count;
};
