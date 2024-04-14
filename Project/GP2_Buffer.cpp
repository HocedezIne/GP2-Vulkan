#include "GP2_Buffer.h"

#include <vulkanbase/VulkanBase.h>

GP2_Buffer::GP2_Buffer(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) :
	m_VkDevice(context.device), m_VkPhysicalDevice(context.physicalDevice), m_Size(size)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_Size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_VkDevice, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("faild to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VkDevice, m_Buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(m_VkPhysicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(m_VkDevice, m_Buffer, m_BufferMemory, 0);
}

void GP2_Buffer::Destroy()
{
	vkDestroyBuffer(m_VkDevice, m_Buffer, nullptr);
	vkFreeMemory(m_VkDevice, m_BufferMemory, nullptr);
}

void GP2_Buffer::MapMemory(void* data)
{
	void* diff;
	vkMapMemory(m_VkDevice, m_BufferMemory, 0, m_Size, 0, &diff);
	memcpy(diff, data, (size_t)m_Size);
	vkUnmapMemory(m_VkDevice, m_BufferMemory);
}

void GP2_Buffer::CopyData(QueueFamilyIndices queueFamInd, GP2_Buffer sourceBuffer, VkQueue graphicsQueue)
{
	GP2_CommandPool cmdPool{};
	cmdPool.Initialize(m_VkDevice, queueFamInd);
	GP2_CommandBuffer cmdBuffer = cmdPool.CreateCommandBuffer();

	cmdBuffer.BeginRecording();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = m_Size;
	vkCmdCopyBuffer(cmdBuffer.GetVkCommandBuffer(), sourceBuffer.GetVkBuffer(), m_Buffer, 1, &copyRegion);

	cmdBuffer.EndRecording();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	cmdBuffer.Submit(submitInfo);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	auto rawBuffer = cmdBuffer.GetVkCommandBuffer();
	vkFreeCommandBuffers(m_VkDevice, cmdPool.GetVkCommandPool(), 1, &rawBuffer);
	cmdPool.Destroy();
}

void GP2_Buffer::BindAsVertexBuffer(VkCommandBuffer cmdBuffer)
{
	VkBuffer vertexBuffers[] = { m_Buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
}

void GP2_Buffer::BindAsIndexBuffer(VkCommandBuffer cmdBuffer)
{
	vkCmdBindIndexBuffer(cmdBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT16);
}

VkBuffer GP2_Buffer::GetVkBuffer() const
{
	return m_Buffer;
}

VkDeviceSize GP2_Buffer::GetSizeInBytes() const
{
	return m_Size;
}
