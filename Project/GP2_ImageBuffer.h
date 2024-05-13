#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkanbase/VulkanUtil.h>

#include "GP2_Buffer.h"

class GP2_ImageBuffer
{
public:
	GP2_ImageBuffer(const VulkanContext& context);
	~GP2_ImageBuffer() = default;

	void LoadImageData(const std::string& filePath, const VulkanContext& context);
	void Initialize(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageAspectFlags aspectFlags);

	static VkImageView GP2_ImageBuffer::createImageViewStatic(VkDevice Vkdevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkImageView GetView() const { return m_ImageView; };
	VkSampler GetSampler() const { return m_Sampler; };

	void Destroy();

private:
	void CreateImage();
	void TransitionLayout(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue);

	void CreateSampler();

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	int m_ImageWidth{};
	int m_ImageHeight{};
	int m_ImageChannels{};

	VkImage m_Image{};
	VkDeviceMemory m_ImageMemory{};
	VkImageView m_ImageView{};
	VkSampler m_Sampler{};

	GP2_Buffer* m_StagingBuffer{};

	VkDevice m_VkDevice;
	VkPhysicalDevice m_VkPhysicalDevice;
};
