#pragma once

#include "GP2_PBRBasePipeline.h"

template <class UBO, class Vertex>
class GP2_PBRMetalnessPipeline final : public GP2_PBRBasePipeline<UBO, Vertex>
{
public:
	GP2_PBRMetalnessPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~GP2_PBRMetalnessPipeline() = default;

	void SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal,
		const std::string& metalness, const std::string& roughness, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue);

	virtual void Initialize(const VulkanContext& context, size_t descriptorPoolCount);
	virtual void CleanUp() override;

private:
	GP2_ImageBuffer* m_DiffuseMap;
	GP2_ImageBuffer* m_NormalMap;
	GP2_ImageBuffer* m_MetalnessMap;
	GP2_ImageBuffer* m_RoughnessMap;
};

template<class UBO, class Vertex>
void GP2_PBRMetalnessPipeline<UBO, Vertex>::SetTextureMaps(const VulkanContext& context, const std::string& diffuse, const std::string& normal,
	const std::string& metalness, const std::string& roughness, QueueFamilyIndices queueFamInd, VkQueue graphicsQueue)
{
	m_DiffuseMap = new GP2_ImageBuffer{ context };
	m_DiffuseMap->LoadImageData(diffuse, context);
	m_DiffuseMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	m_NormalMap = new GP2_ImageBuffer{ context };
	m_NormalMap->LoadImageData(normal, context);
	m_NormalMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	m_MetalnessMap = new GP2_ImageBuffer{ context };
	m_MetalnessMap->LoadImageData(metalness, context);
	m_MetalnessMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	m_RoughnessMap = new GP2_ImageBuffer{ context };
	m_RoughnessMap->LoadImageData(roughness, context);
	m_RoughnessMap->Initialize(queueFamInd, graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

template <class UBO, class Vertex>
void GP2_PBRMetalnessPipeline<UBO, Vertex>::CleanUp()
{
	m_DiffuseMap->Destroy();
	m_DiffuseMap = nullptr;
	m_NormalMap->Destroy();
	m_NormalMap = nullptr;
	m_MetalnessMap->Destroy();
	m_MetalnessMap = nullptr;
	m_RoughnessMap->Destroy();
	m_RoughnessMap = nullptr;

	GP2_PBRBasePipeline<UBO, Vertex>::CleanUp();
}

template <class UBO, class Vertex>
void GP2_PBRMetalnessPipeline<UBO, Vertex>::Initialize(const VulkanContext& context, size_t descriptorPoolCount)
{
	std::vector<std::pair<VkImageView, VkSampler>> imageDatas;
	imageDatas.push_back(std::make_pair(m_DiffuseMap->GetView(), m_DiffuseMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_NormalMap->GetView(), m_NormalMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_MetalnessMap->GetView(), m_MetalnessMap->GetSampler()));
	imageDatas.push_back(std::make_pair(m_RoughnessMap->GetView(), m_RoughnessMap->GetSampler()));

	m_DescriptorPool = new GP2_DescriptorPool<UBO>{ context.device, descriptorPoolCount, imageDatas.size() };
	m_DescriptorPool->Initialize(context, imageDatas.size());
	m_DescriptorPool->CreateDescriptorSets(imageDatas);

	GP2_PBRBasePipeline<UBO, Vertex>::Initialize(context);
}

template <class UBO, class Vertex>
GP2_PBRMetalnessPipeline<UBO, Vertex>::GP2_PBRMetalnessPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	GP2_PBRBasePipeline<UBO, Vertex>(vertexShaderFile, fragmentShaderFile)
{ }