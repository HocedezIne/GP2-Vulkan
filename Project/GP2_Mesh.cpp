#include "GP2_Mesh.h"

#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

void GP2_Mesh::Initialize(VkDevice device, VkPhysicalDevice physicalDevice, GP2_CommandBuffer cmdBuffer, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_VkDevice = device;
	m_CommandBuffer = cmdBuffer;

	GP2_Buffer stagingBuffer{device, physicalDevice, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
	stagingBuffer.MapMemory(m_Vertices.data());
	m_VertexBuffer = new GP2_Buffer{ device, physicalDevice, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
	m_VertexBuffer->CopyData(queueFamInd, stagingBuffer, graphicsQueue);
	stagingBuffer.Destroy();
}

void GP2_Mesh::DestroyMesh()
{
	m_VertexBuffer->Destroy();
	delete m_VertexBuffer;
}

void GP2_Mesh::Draw()
{
	m_VertexBuffer->BindAsVertexBuffer(m_CommandBuffer.GetVkCommandBuffer());
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