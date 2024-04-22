#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "CommandBuffer.h"
#include "GP2_Buffer.h"
#include "GP2_Vertex.h"

class GP2_Mesh
{
public:
	GP2_Mesh() = default;
	~GP2_Mesh() = default;

	void Initialize(const VulkanContext& context, GP2_CommandBuffer cmdBuffer, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue);
	void DestroyMesh();

	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer cmdBuffer);

	void AddVertex(const glm::vec2& pos, const glm::vec3& color);
	void AddVertex(const glm::vec3& pos, const glm::vec3& color);
	void AddVertex(std::vector<GP2_Vertex> vertices);
	void AddIndex(uint16_t index);
	void AddIndex(std::vector<uint16_t> indices);

	bool ParseOBJ(const std::string& filename, bool flipAxisAndWinding = true);

private:	
	GP2_Buffer* m_VertexBuffer{};
	GP2_Buffer* m_IndexBuffer{};

	std::vector<GP2_Vertex> m_Vertices{};
	std::vector<uint16_t> m_Indices{};

	VkDevice m_VkDevice{ VK_NULL_HANDLE };

	GP2_MeshData m_VertexConstant{ glm::mat4(1.f) };
};
