#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "VulkanUtil.h"

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

#include "GP2_Shader.h"
#include "GP2_CommandPool.h"
#include "GP2_Mesh.h"
#include "GP2_DescriptorPool.h"
#include "GP2_Buffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
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
	glm::vec2 m_DragStart{ 0,0 };
	float m_Rotation{};
	float m_Radius{ 10.f };

	glm::vec3 m_CameraPos{ 2.f,2.f,2.f };
	glm::vec3 m_CameraForward{0.f, 0.f, 1.f};
	glm::vec3 m_CameraRight{ 1.f, 0.f, 0.f };
	const glm::vec3 m_CameraUp{ 0.f,1.f,0.f };
	const float m_FovAngle{ 45.f };
	const float m_AspectRatio{ swapChainExtent.width / (float)swapChainExtent.height };

	glm::vec3 m_TargetPos{};
	const glm::vec3 m_UpVector{};

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

		m_CommandPool.Initialize(device, findQueueFamilies(physicalDevice));
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer();
		
		// week 03
		m_GradientShader.Initialize(device);

		m_DescriptorPool = new GP2_DescriptorPool{ device, MAX_FRAMES_IN_FLIGHT };

		/*m_TriangleMesh.AddVertex({ 0.f, -0.5f, 0.f }, { 1.f, 1.f, 1.f });
		m_TriangleMesh.AddVertex({ 0.5f, 0.5f, 0.f }, { 0.f, 1.f, 0.f });
		m_TriangleMesh.AddVertex({ -0.5f, 0.5f, 0.f }, { 0.f, 0.f, 1.f });
		m_TriangleMesh.AddIndex({ 2,1,0 });
		m_TriangleMesh.Initialize(VulkanContext{device, physicalDevice, renderPass, swapChainExtent}, m_CommandBuffer, findQueueFamilies(physicalDevice), graphicsQueue);*/

		m_RectMesh.AddVertex({
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
			});
		//m_RectMesh.AddVertex({ 0.25f, -0.75f, 0.f }, { 1.f, 0.f, 0.f });
		//m_RectMesh.AddVertex({ 0.75f, -0.5f, 0.f }, { 1.f, 1.f, 0.f });
		//m_RectMesh.AddVertex({ 0.75f, -0.75f, 0.f }, { 1.f, 1.f, 1.f });
		m_RectMesh.AddIndex({ 0,1,2,2,3,0 });
		m_RectMesh.Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, m_CommandBuffer, findQueueFamilies(physicalDevice), graphicsQueue);

		//m_OvalMesh.AddVertex({ -0.625f, -0.625f, 0.f }, { 1.f, 1.f, 0.f }); // 0
		//m_OvalMesh.AddVertex({ -0.625f, -0.375f, 0.f }, { 0.f, 1.f, 0.f }); // 1
		//m_OvalMesh.AddVertex({ -0.5f, -0.25f, 0.f }, { 0.f, 1.f, 1.f }); // 2
		//m_OvalMesh.AddVertex({ -0.375f, -0.375f, 0.f }, { 0.f, 0.f, 1.f }); // 3
		//m_OvalMesh.AddVertex({ -0.375f, -0.625f, 0.f }, { 1.f, 0.f, 1.f }); // 4
		//m_OvalMesh.AddVertex({ -0.5f, -0.75f, 0.f }, { 1.f,0.f,0.f }); // 5
		//m_OvalMesh.AddVertex({ -0.5f, -0.5f, 0.f }, { 1.f,1.f,1.f }); // center, 6
		//m_OvalMesh.AddIndex({ 1,6,0,2,6,1,3,6,2,4,6,3,5,6,4,0,6,5 });
		//m_OvalMesh.Initialize(VulkanContext{ device, physicalDevice, renderPass, swapChainExtent }, m_CommandBuffer, findQueueFamilies(physicalDevice), graphicsQueue);

		m_ParsedMesh.ParseOBJ("D:\\DAE\\sm4\\graphics prog2\\Resources\\simple_cube.obj", false);
		m_ParsedMesh.Initialize(device, physicalDevice, m_CommandBuffer, findQueueFamilies(physicalDevice), graphicsQueue);

		createRenderPass();
		createGraphicsPipeline();
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

		//m_TriangleMesh.DestroyMesh();
		m_RectMesh.DestroyMesh();
		m_OvalMesh.DestroyMesh();
		m_ParsedMesh.DestroyMesh();

		m_CommandPool.Destroy();

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);

		delete m_DescriptorPool;

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

	const size_t MAX_FRAMES_IN_FLIGHT = 1;
	const int CURRENT_FRAME = 0;

	GP2_Shader m_GradientShader{ "shaders/shader.vert.spv", "shaders/shader.frag.spv" };
	GP2_DescriptorPool* m_DescriptorPool;

	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
		ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		ubo.proj = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.f);
		ubo.proj[1][1] *= -1;

		m_DescriptorPool->SetUBO(ubo, currentImage);
	}

	// Week 01: 
	// Actual window
	// simple fragment + vertex shader creation functions
	// These 5 functions should be refactored into a separate C++ class
	// with the correct internal state.

	GLFWwindow* window;
	void initWindow();

	void drawScene();

	// Week 02
	// Queue families
	// CommandBuffer concept

	GP2_CommandPool m_CommandPool;
	GP2_CommandBuffer m_CommandBuffer;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void drawFrame(uint32_t imageIndex);
	
	// Week 03
	// Renderpass concept
	// Graphics pipeline
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkRenderPass renderPass;

	GP2_GraphicsPipeline2D<GP2_ViewProjection> m_GP2D{ "shaders/shader.vert.spv", "shaders/shader.frag.spv" };
	GP2_GraphicsPipeline3D<UniformBufferObject> m_GP3D{ "shaders/3Dshader.vert.spv", "shaders/3Dshader.frag.spv" };

	void createFrameBuffers();
	void createRenderPass();
	void createGraphicsPipeline();

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