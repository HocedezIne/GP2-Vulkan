#pragma once

#include <vulkanbase/VulkanUtil.h>
#include <vulkan/vulkan_core.h>
#include <fstream>
#include "3rdParty/json.hpp"

#include "GP2_UniformBufferObject.h"

#include "GP2_PBRMetalnessPipeline.h"
#include "GP2_PBRSpecularPipeline.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


using json = nlohmann::json;

struct Object {
    bool parse_file;
    std::string file;
    std::vector<std::vector<float>> vertex_list;
    std::vector<int> index_list;
};

struct Pipeline {
    std::string pipeline;
    std::string vertex_file;
    std::string fragment_file;
    std::vector<Object> objects;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

static std::vector<GP2_PBRBasePipeline<UniformBufferObject, GP2_PBRVertex>* > parseScene(const std::string& file, const VulkanContext& context, GP2_CommandBuffer cmdBuffer, QueueFamilyIndices queueFam,
    VkQueue graphicsQueue, int maxFrames)
{
    std::vector<GP2_PBRBasePipeline<UniformBufferObject, GP2_PBRVertex>* > createdPipelines;

	std::ifstream f(file);

    if (f.is_open())
    {
        json j;
        f >> j;

        for (const json& pipeline : j["pipelines"])
        {
            if (pipeline["pipeline"].get<std::string>() == "PBRMetalness")
                createdPipelines.push_back(new GP2_PBRMetalnessPipeline<UniformBufferObject, GP2_PBRVertex>{
                    pipeline["vertex file"], pipeline["fragment file"] });
            else if (pipeline["pipeline"].get<std::string>() == "PBRSpecular")
                createdPipelines.push_back(new GP2_PBRSpecularPipeline<UniformBufferObject, GP2_PBRVertex>{
                    pipeline["vertex file"], pipeline["fragment file"] });
            else throw std::invalid_argument("unknown pipeline type to parser");

            for (const json& meshj : pipeline["objects"])
            {
                auto mesh = std::make_unique<GP2_Mesh<GP2_PBRVertex>>();
                mesh->ParseOBJ(meshj["file"], meshj["winding"]);
                mesh->Initialize(context, cmdBuffer, queueFam, graphicsQueue);

                auto model = glm::translate(glm::mat4{ 1.f }, glm::vec3{ meshj["translation"][0], meshj["translation"][1] , meshj["translation"][2] });
                model = glm::rotate(model, glm::radians(meshj["rotation angle"].get<float>()), glm::vec3{meshj["rotation axis"][0], meshj["rotation axis"][1], meshj["rotation axis"][2]});
                model = glm::scale(model, glm::vec3{ meshj["scale"][0], meshj["scale"][1], meshj["scale"][2] });
                mesh->SetVertexConstant(model);

                createdPipelines[createdPipelines.size() - 1]->AddMesh(std::move(mesh));
            }

            createdPipelines[createdPipelines.size() - 1]->SetTextureMaps(context, pipeline["texture files"][0], pipeline["texture files"][1], 
                pipeline["texture files"][2], pipeline["texture files"][3], queueFam, graphicsQueue);
            createdPipelines[createdPipelines.size() - 1]->Initialize(context, maxFrames);
        }

        f.close();
    }
    else std::cerr << "error with json file: " << file << std::endl;

    return createdPipelines;
}