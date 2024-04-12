#pragma once
#include <vulkan/vulkan_core.h>

#include "GP2_Buffer.h"

template <class UBO>
class GP2_UniformBufferObject
{
public:
	void Initialize(VkDevice device, VkPhysicalDevice physicalDevice);
	void Upload();
	void SetData(UBO ubo) { m_UBOSrc = ubo; };

	VkBuffer getVkBuffer() { return m_UBOBuffer->GetVkBuffer(); };

private:
	std::unique_ptr<GP2_Buffer> m_UBOBuffer;
	UBO m_UBOSrc;
};

template<class UBO>
using GP2_UniformBufferObjectPtr = std::unique_ptr<GP2_UniformBufferObject<UBO>>;