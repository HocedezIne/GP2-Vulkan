#pragma once
#include "vulkan/vulkan_core.h"
#include "CommandBuffer.h"

struct QueueFamilyIndices;

class GP2_CommandPool
{
public:
	GP2_CommandPool() = default;
	~GP2_CommandPool() = default;

	void Initialize(const VkDevice& device, const QueueFamilyIndices& queue);
	void Destroy();

	GP2_CommandBuffer CreateCommandBuffer() const;
	VkCommandPool GetVkCommandPool() const;

private:
	VkCommandPool m_CommandPool{ VK_NULL_HANDLE };
	VkDevice m_VkDevice{ VK_NULL_HANDLE };
};
