#include "GP2_Mesh.h"

#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

#include <filesystem>

void GP2_Mesh::Initialize(const VulkanContext& context, GP2_CommandBuffer cmdBuffer, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_VkDevice = context.device;

	GP2_Buffer stagingVertexBuffer{context, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
	stagingVertexBuffer.UploadMemoryData(m_Vertices.data());
	m_VertexBuffer = new GP2_Buffer{ context, sizeof(m_Vertices[0]) * m_Vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
	m_VertexBuffer->CopyData(queueFamInd, stagingVertexBuffer, graphicsQueue);
	stagingVertexBuffer.Destroy();

	GP2_Buffer stagingIndexBuffer{ context, sizeof(m_Indices[0]) * m_Indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	stagingIndexBuffer.UploadMemoryData(m_Indices.data());
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

void GP2_Mesh::AddVertex(const glm::vec2& pos, const glm::vec3& color)
{
	m_Vertices.push_back(GP2_Vertex{ {pos, 0.f}, color });
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

bool GP2_Mesh::ParseOBJ(const std::string& filename, bool flipAxisAndWinding)
{
	auto path = std::filesystem::current_path();
	std::cout << path << std::endl;

	std::ifstream file(filename);
	if (!file)
		return false;

	std::vector<glm::vec3> positions{};
	//std::vector<glm::vec3> normals{};
	//std::vector<glm::vec3> colors{};
	std::vector<glm::vec2> UVs{};
	bool hasUVs{ false };

	m_Vertices.clear();
	m_Indices.clear();

	std::string sCommand;

	// start a while iteration ending when the end of file is reached (ios::eof)
	while (!file.eof())
	{
		//read the first word of the string, use the >> operator (istream::operator>>) 
		file >> sCommand;

		//use conditional statements to process the different commands	
		if (sCommand == "#")
		{
			// Ignore Comment
		}
		else if (sCommand == "v")
		{
			//Vertex
			float x, y, z;
			file >> x >> y >> z;

			positions.emplace_back(x, y, z);
		}
		else if (sCommand == "vt")
		{
			if (!hasUVs) hasUVs = true;
			// Vertex TexCoord
			float u, v;
			file >> u >> v;
			
			UVs.emplace_back(u, 1 - v);
		}
		else if (sCommand == "vn")
		{
			// Vertex Normal
			//float x, y, z;
			//file >> x >> y >> z;

			// TODO implement normals
			//normals.emplace_back(x, y, z);
		}
		else if (sCommand == "f")
		{
			//if a face is read:
			//construct the 3 vertices, add them to the vertex array
			//add three indices to the index array
			//add the material index as attibute to the attribute array
			//
			// Faces or triangles
			GP2_Vertex vertex{};
			size_t iPosition, iTexCoord, iNormal;

			uint32_t tempIndices[3]{};
			for (size_t iFace = 0; iFace < 3; iFace++)
			{
				// OBJ format uses 1-based arrays
				file >> iPosition;
				//vertex.pos = positions[iPosition - 1];
				//vertex.color = { 1.f, 1.f, 1.f };

				if ('/' == file.peek())//is next in buffer ==  '/' ?
				{
					file.ignore();//read and ignore one element ('/')

					if ('/' != file.peek())
					{
						// Optional texture coordinate
						file >> iTexCoord;
						// TODO parse UV coords
						vertex.texCoord = UVs[iTexCoord - 1];
					}

					if ('/' == file.peek())
					{
						file.ignore();

						// Optional vertex normal
						file >> iNormal;
						// TODO parse normals
						//vertex.normal = normals[iNormal - 1];
					}
				}

				//m_Vertices.push_back(vertex);
				tempIndices[iFace] = iPosition-1;
			}

			m_Indices.push_back(tempIndices[0]);
			if (flipAxisAndWinding)
			{
				m_Indices.push_back(tempIndices[2]);
				m_Indices.push_back(tempIndices[1]);
			}
			else
			{
				m_Indices.push_back(tempIndices[1]);
				m_Indices.push_back(tempIndices[2]);
			}
		}
		//read till end of line and ignore all remaining chars
		file.ignore(1000, '\n');
	}

	for (int i{}; i < positions.size(); ++i)
	{
		if(hasUVs) m_Vertices.emplace_back(GP2_Vertex{ positions[i],{1.f,1.f,1.f}, UVs[i] });
		else m_Vertices.emplace_back(GP2_Vertex{ positions[i],{1.f,1.f,1.f}, {1.f,1.f} });
	}

	return true;
}