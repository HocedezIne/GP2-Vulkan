#include "GP2_Mesh.h"

#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

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
	//auto path = std::filesystem::current_path();

	std::ifstream file(filename);
	if (!file)
		return false;

	std::vector<glm::vec3> positions{};
	//std::vector<glm::vec3> normals{};
	std::vector<glm::vec3> colors{};
	//std::vector<glm::vec2> UVs{};

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
			// Vertex TexCoord
			float u, v;
			file >> u >> v;
			// TODO implement UVs
			//UVs.emplace_back(u, 1 - v);
			colors.emplace_back(u, 1 - v, 1.f);
		}
		else if (sCommand == "vn")
		{
			// Vertex Normal
			float x, y, z;
			file >> x >> y >> z;

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
				vertex.pos = positions[iPosition - 1];
				vertex.color = { 1.f, 1.f, 1.f }; // TODO remove once using UV's 

				if ('/' == file.peek())//is next in buffer ==  '/' ?
				{
					file.ignore();//read and ignore one element ('/')

					if ('/' != file.peek())
					{
						// Optional texture coordinate
						file >> iTexCoord;
						// TODO parse UV coords
						//vertex.uv = UVs[iTexCoord - 1];
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

				m_Vertices.push_back(vertex);
				tempIndices[iFace] = uint32_t(m_Vertices.size()) - 1;
				//indices.push_back(uint32_t(vertices.size()) - 1);
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

	// TODO tangents
	////Cheap Tangent Calculations
	//for (uint32_t i = 0; i < m_Indices.size(); i += 3)
	//{
	//	uint32_t index0 = m_Indices[i];
	//	uint32_t index1 = m_Indices[size_t(i) + 1];
	//	uint32_t index2 = m_Indices[size_t(i) + 2];

	//	const glm::vec3& p0 = m_Vertices[index0].pos;
	//	const glm::vec3& p1 = m_Vertices[index1].pos;
	//	const glm::vec3& p2 = m_Vertices[index2].pos;
	//	const Vector2& uv0 = vertices[index0].uv;
	//	const Vector2& uv1 = vertices[index1].uv;
	//	const Vector2& uv2 = vertices[index2].uv;

	//	const Vector3 edge0 = p1 - p0;
	//	const Vector3 edge1 = p2 - p0;
	//	const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
	//	const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
	//	float r = 1.f / Vector2::Cross(diffX, diffY);

	//	Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
	//	vertices[index0].tangent += tangent;
	//	vertices[index1].tangent += tangent;
	//	vertices[index2].tangent += tangent;
	//}

	//////Create the Tangents (reject)
	//for (auto& v : vertices)
	//{
	//	v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

	//	if (flipAxisAndWinding)
	//	{
	//		v.position.z *= -1.f;
	//		v.normal.z *= -1.f;
	//		v.tangent.z *= -1.f;
	//	}

	//}

	return true;
}