#pragma once
//#include <vulkan/vulkan_core.h>
//#include <vulkanbase/VulkanUtil.h>

#include "GP2_ImageBuffer.h"

//#include <vulkanbase/VulkanBase.h>
#include <vector>

static VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

class GP2_DepthBuffer
{
public:
	GP2_DepthBuffer() = default;
	~GP2_DepthBuffer() = default;

	void Initialize(const VulkanContext& context, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue);
	void Destroy();

	VkImageView GetDepthImageView() const { return m_DepthImageView; };

private:
	void CreateDepthImage(int width, int height, VkFormat format);
	void TransitionLayout(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	
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

	bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkDevice m_VkDevice{VK_NULL_HANDLE};
	VkPhysicalDevice m_VkPhysicalDevice{ VK_NULL_HANDLE };
	VkExtent2D m_VkExtent{};

	VkFormat m_DepthFormat{};
	VkImage m_DepthImage{};
	VkDeviceMemory m_DepthImageMemory{};
	VkImageView m_DepthImageView{};
};
