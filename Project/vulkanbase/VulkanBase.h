#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "VulkanUtil.h"

#include "stb_image.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include "jsonParser.h"

#include "GP2_GraphicsPipeline2D.h"
#include "GP2_GraphicsPipeline3D.h"
#include "GP2_PBRSpecularPipeline.h"
#include "GP2_PBRMetalnessPipeline.h"
#include "GP2_UniformBufferObject.h"
#include "GP2_DepthBuffer.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanBase {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	// camera stuff
	void keyEvent(int key, int scancode, int action, int mods);
	void mouseMove(GLFWwindow* window, double xpos, double ypos);
	void mouseEvent(GLFWwindow* window, int button, int action, int mods);

	glm::vec2 m_PrevMousePos{};

	glm::vec3 m_CameraPos{ 0.f, 5.f, 50.f };
	glm::vec3 m_CameraForward{0.f, 0.f, -1.f};
	glm::vec3 m_CameraRight{ 1.f, 0.f, 0.f };
	glm::vec3 m_CameraUp{ 0.f,1.f,0.f };
	float m_Yaw{ 0.f };
	float m_Pitch{ 0.f };
	const float m_CameraMovementSpeed{50.f};
	const float m_MouseSensitivity{ 0.001f };

	const float m_FovAngle{ 45.f };
	const float m_AspectRatio{ swapChainExtent.width / (float)swapChainExtent.height };

	glm::mat4 UpdateCamera();

	void initVulkan() {
		// week 06
		createInstance();
		setupDebugMessenger();
		createSurface();

		// week 05
		pickPhysicalDevice();
		createLogicalDevice();

		// week 04 
		createSwapChain();
		createImageViews();

		auto queueFam = findQueueFamilies(physicalDevice);

		m_CommandPool.Initialize(device, queueFam);
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer();

		m_DepthBuffer.Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, queueFam, graphicsQueue);

		std::unique_ptr<GP2_Mesh<GP2_2DVertex>> m_TriangleMesh = std::make_unique<GP2_Mesh<GP2_2DVertex>>();
		m_TriangleMesh->AddVertex({ GP2_2DVertex{ { 0.f, -0.5f, 0.f }, { 1.f, 1.f, 1.f }},
			GP2_2DVertex{ { 0.5f, 0.5f, 0.f }, { 0.f, 1.f, 0.f }},
			GP2_2DVertex{ { -0.5f, 0.5f, 0.f }, { 0.f, 0.f, 1.f }} });
		m_TriangleMesh->AddIndex({ 2,1,0 });
		m_TriangleMesh->Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, m_CommandBuffer, queueFam, graphicsQueue);
		m_GP2D.AddMesh(std::move(m_TriangleMesh));

		std::unique_ptr<GP2_Mesh<GP2_2DVertex>> m_FlatRectMesh = std::make_unique<GP2_Mesh<GP2_2DVertex>>();
		m_FlatRectMesh->AddVertex({ GP2_2DVertex{ { 0.25f, -0.5f, 0.f }, { 1.f, 0.5f, 1.f }},
			GP2_2DVertex{ {0.25f, -0.75f, 0.f}, { 1.f, 0.f, 0.f}},
			GP2_2DVertex{ {0.75f, -0.5f, 0.f}, { 1.f, 1.f, 0.f}},
			GP2_2DVertex{ {0.75f, -0.75f, 0.f}, {1.f, 1.f, 1.f}} });
		m_FlatRectMesh->AddIndex({ 2,1,0,3,1,2 });
		m_FlatRectMesh->Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, m_CommandBuffer, findQueueFamilies(physicalDevice), graphicsQueue);
		m_GP2D.AddMesh(std::move(m_FlatRectMesh));

		createRenderPass();

		m_GP2D.Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, MAX_FRAMES_IN_FLIGHT);
		m_GP3D.Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, MAX_FRAMES_IN_FLIGHT,
			"resources/vehicle_diffuse.png", queueFam, graphicsQueue);

		m_PBRPipelines = parseScene("resources/scene.json", VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, m_CommandBuffer,
			queueFam, graphicsQueue, MAX_FRAMES_IN_FLIGHT);

		createFrameBuffers();

		// week 06
		createSyncObjects();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			//drawFrame week 06
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}

	void cleanup() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		m_CommandPool.Destroy();

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		m_DepthBuffer.Destroy();

		m_GP2D.CleanUp();
		m_GP3D.CleanUp();

		for (auto& pipeline : m_PBRPipelines)
		{
			pipeline->CleanUp();
		}

		vkDestroyRenderPass(device, renderPass, nullptr);

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);

		vkDestroyDevice(device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	GP2_DepthBuffer m_DepthBuffer{};

	const size_t MAX_FRAMES_IN_FLIGHT = 1;
	const int CURRENT_FRAME = 0;

	// Week 01: 
	// Actual window

	GLFWwindow* window;
	void initWindow();

	// Week 02
	// Queue families
	// CommandBuffer concept

	GP2_CommandPool m_CommandPool;
	GP2_CommandBuffer m_CommandBuffer;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	
	// Week 03
	// Renderpass concept
	// Graphics pipeline
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;

	GP2_GraphicsPipeline2D<GP2_ViewProjection, GP2_2DVertex> m_GP2D{ "shaders/shader.vert.spv", "shaders/shader.frag.spv" };
	GP2_GraphicsPipeline3D<UniformBufferObject, GP2_3DVertex> m_GP3D{ "shaders/3Dshader.vert.spv", "shaders/3Dshader.frag.spv" };
	std::vector<GP2_PBRBasePipeline<UniformBufferObject, GP2_PBRVertex>* > m_PBRPipelines;

	void createFrameBuffers();
	void createRenderPass();

	void beginRenderPass(const GP2_CommandBuffer& cmdBuffer, VkFramebuffer currentBuffer, VkExtent2D extent);
	void endRenderPass(const GP2_CommandBuffer& cmdBuffer);

	// Week 04
	// Swap chain and image view support

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImageView> swapChainImageViews;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	void createSwapChain();
	void createImageViews();

	// Week 05 
	// Logical and physical device

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	// Week 06
	// Main initialization

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector <VkSemaphore> renderFinishedSemaphores;
	std::vector <VkFence> inFlightFences;

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();

	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};