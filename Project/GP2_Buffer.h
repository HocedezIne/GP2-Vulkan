#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkanbase/VulkanUtil.h>
#include <stdexcept>

#include "GP2_CommandPool.h"


class GP2_Buffer
{
public:
	GP2_Buffer(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	~GP2_Buffer() /*{ Destroy(); }*/ = default;

	void Destroy();

	void UploadMemoryData(void* data);
	void MapMemory(void* data);
	void CopyData(QueueFamilyIndices queueFamInd, GP2_Buffer sourceBuffer, VkQueue graphicsQueue);

	void BindAsVertexBuffer(VkCommandBuffer cmdBuffer);
	void BindAsIndexBuffer(VkCommandBuffer cmdBuffer);

	VkBuffer GetVkBuffer() const;
	VkDeviceSize GetSizeInBytes() const;

private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkDevice m_VkDevice;
	VkPhysicalDevice m_VkPhysicalDevice;

	VkDeviceSize m_Size;

	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;
};