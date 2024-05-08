#include "GP2_DepthBuffer.h"

#include <vulkanbase/VulkanBase.h>
#include "GP2_ImageBuffer.h"

VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

GP2_DepthBuffer::GP2_DepthBuffer(const VulkanContext& context) :
	m_VkDevice(context.device), m_VkPhysicalDevice(context.physicalDevice), m_VkExtent(context.swapChainExtent)
{

}

GP2_DepthBuffer::~GP2_DepthBuffer()
{

}

void GP2_DepthBuffer::Initialize()
{
	m_DepthFormat = findSupportedFormat(m_VkPhysicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	CreateDepthImage(m_VkExtent.width, m_VkExtent.height, m_DepthFormat);
	m_DepthImageView = GP2_ImageBuffer::createImageViewStatic(m_VkDevice, m_DepthImage, m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void GP2_DepthBuffer::Destroy()
{
	vkDestroyImageView(m_VkDevice, m_DepthImageView, nullptr);

	vkDestroyImage(m_VkDevice, m_DepthImage, nullptr);
	vkFreeMemory(m_VkDevice, m_DepthImageMemory, nullptr);
}

void GP2_DepthBuffer::CreateDepthImage(int width, int height, VkFormat format)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &m_DepthImage) != VK_SUCCESS)
		throw std::runtime_error("failed to create image!\n");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, m_DepthImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &m_DepthImageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate image memory\n");

	vkBindImageMemory(m_VkDevice, m_DepthImage, m_DepthImageMemory, 0);
}