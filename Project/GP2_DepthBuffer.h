#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkanbase/VulkanUtil.h>
#include <vector>

static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

class GP2_DepthBuffer
{
public:
	GP2_DepthBuffer(const VulkanContext& context);
	~GP2_DepthBuffer();

	void Initialize();
	void Destroy();

	bool hasStencilComponent()
	{
		return m_DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || m_DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT;
	}

private:
	void CreateDepthImage(int width, int height, VkFormat format);

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

	VkDevice m_VkDevice;
	VkPhysicalDevice m_VkPhysicalDevice;
	VkExtent2D m_VkExtent;

	VkFormat m_DepthFormat{};
	VkImage m_DepthImage{};
	VkDeviceMemory m_DepthImageMemory{};
	VkImageView m_DepthImageView{};
};
