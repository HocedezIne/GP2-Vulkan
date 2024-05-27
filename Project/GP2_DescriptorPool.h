#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>

#include "GP2_Buffer.h"
#include "GP2_Buffer.h"

template<class UBO>
class GP2_DescriptorPool
{
public:
	GP2_DescriptorPool(VkDevice device, size_t count, size_t imageCount = 0);
	~GP2_DescriptorPool();

	void Initialize(const VulkanContext& context, size_t imageCount = 0);

	void SetUBO(UBO data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; };

	void CreateDescriptorSets(std::vector<std::pair<VkImageView, VkSampler>> imageDatas = {});

	void BindDescriptorSet(VkCommandBuffer cmdBuffer, VkPipelineLayout layout, size_t index);

private:
	VkDevice m_Device;
	VkDeviceSize m_Size;
	VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };

	void CreateDescriptorSetLayout(size_t imageCount);
	void CreateUBOs(const VulkanContext& context);

	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet> m_DescriptorSets;

	std::vector<GP2_Buffer*> m_UBOs;
	std::vector<void*> m_UBOsMapped;

	size_t m_Count;
};

template<class UBO>
GP2_DescriptorPool<UBO>::GP2_DescriptorPool(VkDevice device, size_t count, size_t imageCount) :
	m_Device(device), m_Size(sizeof(UBO)), m_Count(count)
{
	std::vector<VkDescriptorPoolSize> poolSizes(imageCount+1);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_Count);

	for (int idx{ 1 }; idx < imageCount + 1; ++idx)
	{
		poolSizes[idx].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[idx].descriptorCount = static_cast<uint32_t>(m_Count);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_Count);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

template<class UBO>
GP2_DescriptorPool<UBO>::~GP2_DescriptorPool()
{
	for (size_t i = 0; i < m_Count; ++i)
	{
		m_UBOs[i]->Destroy();
	}

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::Initialize(const VulkanContext& context, size_t imageCount)
{
	CreateDescriptorSetLayout(imageCount);
	CreateUBOs(context);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::SetUBO(UBO data, size_t index)
{
	memcpy(m_UBOsMapped[index], &data, m_Size);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSets(std::vector<std::pair<VkImageView, VkSampler>> imageDatas)
{
	std::vector<VkDescriptorSetLayout> layouts(m_Count, m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_Count);
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(m_Count);
	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < m_Count; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UBOs[i]->GetVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_Size;

		if (!imageDatas.empty())
		{
			std::vector<VkDescriptorImageInfo> imageInfos(imageDatas.size());
			for (size_t idx = 0; idx < static_cast<int>(imageDatas.size()); ++idx)
			{
				imageInfos[idx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[idx].imageView = imageDatas[idx].first;
				imageInfos[idx].sampler = imageDatas[idx].second;
			}

			std::vector<VkWriteDescriptorSet> descriptorWrites(imageDatas.size() + 1);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_DescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			for (size_t idx = 0; idx < static_cast<int>(imageDatas.size()); ++idx)
			{
				descriptorWrites[1+idx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1+idx].dstSet = m_DescriptorSets[i];
				descriptorWrites[1+idx].dstBinding = static_cast<uint32_t>(1 + idx);
				descriptorWrites[1+idx].dstArrayElement = 0;
				descriptorWrites[1+idx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1+idx].descriptorCount = 1;
				descriptorWrites[1+idx].pImageInfo = &imageInfos[idx];
			}

			vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
		else {
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::BindDescriptorSet(VkCommandBuffer cmdBuffer, VkPipelineLayout layout, size_t index)
{
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &m_DescriptorSets[index], 0, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSetLayout(size_t imageCount)
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings(imageCount + 1);
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = nullptr;

	for (int idx{1}; idx < imageCount+1; ++idx)
	{
		layoutBindings[idx].binding = idx;
		layoutBindings[idx].descriptorCount = 1;
		layoutBindings[idx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[idx].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[idx].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateUBOs(const VulkanContext& context)
{
	m_UBOs.resize(m_Count);
	m_UBOsMapped.resize(m_Count);

	for (size_t i = 0; i < m_Count; ++i)
	{
		m_UBOs[i] = new GP2_Buffer(context, m_Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_UBOs[i]->MapMemory(&m_UBOsMapped[i]);
	}
}