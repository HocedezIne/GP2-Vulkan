#include "GP2_Mesh.h"

#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

void GP2_Mesh::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, GP2_CommandBuffer cmdBuffer)
{
	m_VkDevice = device;
	m_CommandBuffer = cmdBuffer;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(m_Vertices[0]) * m_Vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_VkDevice, &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("faild to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VkDevice, m_VertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_VkDevice, m_VertexBuffer, m_VertexBufferMemory, 0);
}

void GP2_Mesh::DestroyMesh()
{
	vkDestroyBuffer(m_VkDevice, m_VertexBuffer, nullptr);
	vkFreeMemory(m_VkDevice, m_VertexBufferMemory, nullptr);
}

void GP2_Mesh::Draw()
{
	VkBuffer vertexBuffers[] = { m_VertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffer.GetVkCommandBuffer(), 0, 1, vertexBuffers, offsets);

	vkCmdDraw(m_CommandBuffer.GetVkCommandBuffer(), static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
}

void GP2_Mesh::AddVertex(const glm::vec3& pos, const glm::vec3& color)
{
	m_Vertices.push_back(GP2_Vertex{ pos, color });
}

void GP2_Mesh::AddVertex(std::vector<GP2_Vertex> vertices)
{
	m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
}