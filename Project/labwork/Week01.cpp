#include "vulkanbase/VulkanBase.h"

#include <algorithm>

void VulkanBase::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	// camera stuff
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->keyEvent(key, scancode, action, mods);
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->mouseMove(window, xpos, ypos);
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->mouseEvent(window, button, action, mods);
	});
}

void VulkanBase::keyEvent(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos += m_CameraForward * 0.5f;
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos -= m_CameraForward * 0.5f;
	}
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos -= glm::vec3{1.f,0.f,0.f} *5.f;
	}
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos += glm::vec3{ 1.f,0.f,0.f }  * 0.5f;
	}
}

void VulkanBase::mouseMove(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		float dx = static_cast<float>(xpos) - m_DragStart.x;
		if (dx > 0) {
			m_Rotation += 0.01;
		}
		else {
			m_Rotation -= 0.01;
		}
	}
}
void VulkanBase::mouseEvent(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		std::cout << "right mouse button pressed\n";
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		m_DragStart.x = static_cast<float>(xpos);
		m_DragStart.y = static_cast<float>(ypos);
	}
}