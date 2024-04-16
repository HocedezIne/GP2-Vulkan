#include "GP2_Mesh.h"

#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

void GP2_Mesh::Initialize(const VulkanContext& context, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_VkDevice = context.device;

	GP2_Buffer stagingVertexBuffer{context, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
	stagingVertexBuffer.MapMemory(m_Vertices.data());
	m_VertexBuffer = new GP2_Buffer{ context, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
	m_VertexBuffer->CopyData(queueFamInd, stagingVertexBuffer, graphicsQueue);
	stagingVertexBuffer.Destroy();

	GP2_Buffer stagingIndexBuffer{ context, sizeof(m_Indices[0]) * m_Indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	stagingIndexBuffer.MapMemory(m_Indices.data());
	m_IndexBuffer = new GP2_Buffer{ context, sizeof(m_Indices[0]) * m_Indices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
	m_IndexBuffer->CopyData(queueFamInd, stagingIndexBuffer, graphicsQueue);
	stagingIndexBuffer.Destroy();
}

void GP2_Mesh::DestroyMesh()
{
	m_VertexBuffer->Destroy();
	delete m_VertexBuffer;

	m_IndexBuffer->Destroy();
	delete m_IndexBuffer;
}

void GP2_Mesh::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer cmdBuffer)
{
	m_VertexBuffer->BindAsVertexBuffer(cmdBuffer);
	m_IndexBuffer->BindAsIndexBuffer(cmdBuffer);

	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GP2_MeshData), &m_VertexConstant);

	vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
}

void GP2_Mesh::AddVertex(const glm::vec3& pos, const glm::vec3& color)
{
	m_Vertices.push_back(GP2_Vertex{ pos, color });
}

void GP2_Mesh::AddVertex(std::vector<GP2_Vertex> vertices)
{
	m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
}

void GP2_Mesh::AddIndex(uint16_t index)
{
	m_Indices.push_back(index);
}

void GP2_Mesh::AddIndex(std::vector<uint16_t> indices)
{
	m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());
}
