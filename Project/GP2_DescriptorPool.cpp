#include "GP2_DescriptorPool.h"

#include "vulkanbase/VulkanUtil.h"

template<class UBO>
GP2_DescriptorPool<UBO>::GP2_DescriptorPool(VkDevice device, size_t count) :
	m_Device(device), m_Size(sizeof(UBO)), m_Count(count)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(count);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = count;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

template<class UBO>
GP2_DescriptorPool<UBO>::~GP2_DescriptorPool()
{
	for (auto& buffer : m_UBOs)
	{
		buffer.Reset();
	}

	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::Initialize()
{
	CreateDescriptorSetLayout();
	CreateUBOs();
	CreateDescriptorSets();
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!\n");
}

template<class UBO>
void GP2_DescriptorPool<UBO>::BindDescriptorSet(GP2_CommandBuffer cmdBuffer, VkPipelineLayout layout, size_t index)
{
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &m_DescriptorSets[index], 0, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_Count, m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = m_Count;
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(m_Count);
	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}


	size_t descriptorIndex = 0;
	for (DAEUniformBufferObjectPtr<UBO>& buffer : m_UBOs)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer->getVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_Size;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[descriptorIndex];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		++descriptorIndex;
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateUBOs()
{
	for (int uboIndex = 0; uboIndex < m_Count; ++uboIndex)
	{
		DAEUniformBufferObjectPtr<UBO> buffer = std::make_unique<DAEUniformBufferObject<UBO>>();
		buffer->initialize(context);
		m_UBOs.emplace_back(std::move(buffer));
	}
}

template<class UBO>
inline void GP2_DescriptorPool<UBO>::setUBO(UBO src, size_t index)
{
	if (index < m_UBOs.size())
	{
		m_UBOs[index]->setData(src);
		m_UBOs[index]->upload();
	}
}
