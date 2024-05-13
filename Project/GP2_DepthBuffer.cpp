#include "GP2_DepthBuffer.h"

#include <vulkanbase/VulkanBase.h>

void GP2_DepthBuffer::Initialize(const VulkanContext& context, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_VkDevice = context.device;
	m_VkPhysicalDevice = context.physicalDevice;
	m_VkExtent = context.swapChainExtent;

	m_DepthFormat = findSupportedFormat(m_VkPhysicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	CreateDepthImage(m_VkExtent.width, m_VkExtent.height, m_DepthFormat);
	m_DepthImageView = GP2_ImageBuffer::createImageViewStatic(m_VkDevice, m_DepthImage, m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	TransitionLayout(queueFamInd, graphicsQueue, m_DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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

void GP2_DepthBuffer::TransitionLayout(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	// beginsingletimecommands in vulkan tutorial
	GP2_CommandPool cmdPool{};
	cmdPool.Initialize(m_VkDevice, queueFamInd);
	GP2_CommandBuffer cmdBuffer = cmdPool.CreateCommandBuffer();
	cmdBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// the other suff in potentionally the wrong place
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_DepthImage;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		throw std::invalid_argument("unsupported layout transition! \n");

	vkCmdPipelineBarrier(
		cmdBuffer.GetVkCommandBuffer(),
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// end singletimecommands in vulkan tutorial
	cmdBuffer.EndRecording();

	VkSubmitInfo submitInfo{};
	cmdBuffer.Submit(submitInfo);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	auto rawBuffer = cmdBuffer.GetVkCommandBuffer();
	vkFreeCommandBuffers(m_VkDevice, cmdPool.GetVkCommandPool(), 1, &rawBuffer);
	cmdPool.Destroy();
}