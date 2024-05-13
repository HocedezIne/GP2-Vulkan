#include "GP2_ImageBuffer.h"

#include <vulkanbase/VulkanBase.h>

GP2_ImageBuffer::GP2_ImageBuffer(const VulkanContext& context) :
	m_VkDevice(context.device), m_VkPhysicalDevice(context.physicalDevice)
{

}

void GP2_ImageBuffer::Initialize(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageAspectFlags aspectFlags)
{
	CreateImage();

	TransitionLayout(queueFamInd, graphicsQueue, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(m_StagingBuffer->GetVkBuffer(), queueFamInd, graphicsQueue);
	TransitionLayout(queueFamInd, graphicsQueue, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_StagingBuffer->Destroy();
	delete m_StagingBuffer;
	m_StagingBuffer = nullptr;

	m_ImageView = createImageViewStatic(m_VkDevice, m_Image, format, aspectFlags);
	CreateSampler();
}

void GP2_ImageBuffer::TransitionLayout(QueueFamilyIndices queueFamInd, VkQueue graphicsQueue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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
	barrier.image = m_Image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

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

void GP2_ImageBuffer::CopyBufferToImage(VkBuffer buffer, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	// beginsingletimecommands in vulkan tutorial
	GP2_CommandPool cmdPool{};
	cmdPool.Initialize(m_VkDevice, queueFamInd);
	GP2_CommandBuffer cmdBuffer = cmdPool.CreateCommandBuffer();
	cmdBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// other stuff
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = {
		static_cast<uint32_t>(m_ImageWidth),
		static_cast<uint32_t>(m_ImageHeight),
		1
	};

	vkCmdCopyBufferToImage(
		cmdBuffer.GetVkCommandBuffer(),
		buffer,
		m_Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
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

void GP2_ImageBuffer::Destroy()
{
	vkDestroySampler(m_VkDevice, m_Sampler, nullptr);
	vkDestroyImageView(m_VkDevice, m_ImageView, nullptr);

	vkDestroyImage(m_VkDevice, m_Image, nullptr);
	vkFreeMemory(m_VkDevice, m_ImageMemory, nullptr);
}

void GP2_ImageBuffer::LoadImageData(const std::string& filePath, const VulkanContext& context)
{
	stbi_uc* pixels = stbi_load(filePath.c_str(), &m_ImageWidth, &m_ImageHeight, &m_ImageChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_ImageWidth) * m_ImageHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	m_StagingBuffer = new GP2_Buffer{ context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	m_StagingBuffer->UploadMemoryData(pixels);
	stbi_image_free(pixels);
}

void GP2_ImageBuffer::CreateImage()
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(m_ImageWidth);
	imageInfo.extent.height = static_cast<uint32_t>(m_ImageHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &m_Image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image!\n");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, m_Image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate image memory\n");

	vkBindImageMemory(m_VkDevice, m_Image, m_ImageMemory, 0);
}

void GP2_ImageBuffer::CreateSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_VkPhysicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}

VkImageView GP2_ImageBuffer::createImageViewStatic(VkDevice Vkdevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(Vkdevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture image view!\n");
	return imageView;
}