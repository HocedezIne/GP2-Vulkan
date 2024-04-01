#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <stdexcept>

#include "CommandBuffer.h"
#include "GP2_Shader.h"

class GP2_Mesh
{
public:
	GP2_Mesh() = default;
	~GP2_Mesh() = default;

	void Initialize(VkDevice device, VkPhysicalDevice physicalDevice, GP2_CommandBuffer cmdBuffer);
	void DestroyMesh();

	void Draw();

	void AddVertex(const glm::vec3& pos, const glm::vec3& color);
	void AddVertex(std::vector<GP2_Vertex> vertices);

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

private:	
	VkBuffer m_VertexBuffer{ VK_NULL_HANDLE };
	VkDeviceMemory m_VertexBufferMemory{ VK_NULL_HANDLE };

	std::vector<GP2_Vertex> m_Vertices{};

	VkDevice m_VkDevice{ VK_NULL_HANDLE };
	GP2_CommandBuffer m_CommandBuffer{};
};
