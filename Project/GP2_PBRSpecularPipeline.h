#pragma once

#include "GP2_PBRBasePipeline.h"

template <class UBO, class Vertex>
class GP2_PBRSpecularPipeline final : public GP2_PBRBasePipeline<UBO, Vertex>
{
public:
	GP2_PBRSpecularPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~GP2_PBRSpecularPipeline() = default;

	void SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal,
		const std::string& gloss, const std::string& specular, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue) override;

	void Initialize(const VulkanContext& context, size_t descriptorPoolCount) override;
	void CleanUp() override;

private:
	GP2_ImageBuffer* m_DiffuseMap;
	GP2_ImageBuffer* m_NormalMap;
	GP2_ImageBuffer* m_GlossMap;
	GP2_ImageBuffer* m_SpecularMap;
};

template<class UBO, class Vertex>
void GP2_PBRSpecularPipeline<UBO, Vertex>::SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal,
	const std::string& gloss, const std::string& specular, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_DiffuseMap = new GP2_ImageBuffer{ context };
	m_DiffuseMap->LoadImageData(diffuse, context);
	m_DiffuseMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	m_NormalMap = new GP2_ImageBuffer{ context };
	m_NormalMap->LoadImageData(normal, context);
	m_NormalMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	m_GlossMap = new GP2_ImageBuffer{ context };
	m_GlossMap->LoadImageData(gloss, context);
	m_GlossMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	m_SpecularMap = new GP2_ImageBuffer{ context };
	m_SpecularMap->LoadImageData(specular, context);
	m_SpecularMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

template <class UBO, class Vertex>
void GP2_PBRSpecularPipeline<UBO, Vertex>::CleanUp()
{
	m_DiffuseMap->Destroy();
	m_DiffuseMap = nullptr;
	m_NormalMap->Destroy();
	m_NormalMap = nullptr;
	m_GlossMap->Destroy();
	m_GlossMap = nullptr;
	m_SpecularMap->Destroy();
	m_SpecularMap = nullptr;

	GP2_PBRBasePipeline<UBO, Vertex>::CleanUp();
}

template <class UBO, class Vertex>
void GP2_PBRSpecularPipeline<UBO, Vertex>::Initialize(const VulkanContext& context, size_t descriptorPoolCount)
{
	std::vector<std::pair<VkImageView, VkSampler>> imageDatas;
	imageDatas.push_back(std::make_pair(m_DiffuseMap->GetView(), m_DiffuseMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_NormalMap->GetView(), m_NormalMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_GlossMap->GetView(), m_GlossMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_SpecularMap->GetView(), m_SpecularMap->GetSampler()));

	m_DescriptorPool = new GP2_DescriptorPool<UBO>{ context.device, descriptorPoolCount, imageDatas.size()};
	m_DescriptorPool->Initialize(context, imageDatas.size());
	m_DescriptorPool->CreateDescriptorSets(imageDatas);

	GP2_PBRBasePipeline<UBO, Vertex>::Initialize(context, descriptorPoolCount);
}

template <class UBO, class Vertex>
GP2_PBRSpecularPipeline<UBO, Vertex>::GP2_PBRSpecularPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	GP2_PBRBasePipeline<UBO, Vertex>(vertexShaderFile, fragmentShaderFile)
{ }