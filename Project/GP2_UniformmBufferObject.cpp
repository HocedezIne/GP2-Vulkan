#include "vulkanbase/VulkanUtil.h"

#include "GP2_UniformmBufferObject.h"

template<class UBO>
inline void GP2_UniformBufferObject<UBO>::Initialize(VkDevice device, VkPhysicalDevice physicalDevice)
{
	m_UBOBuffer = std::make_unique<GP2_DataBuffer>(device, physicaldevice,sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

template<class UBO>
inline void GP2_UniformBufferObject<UBO>::Upload()
{
	m_UBOBuffer->Upload(sizeof(UBO), &m_UBOSrc);
}
