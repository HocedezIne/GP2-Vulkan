#include "vulkanbase/VulkanBase.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

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
		m_CameraPos += m_CameraForward * m_CameraMovementSpeed;
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
		m_CameraPos -= m_CameraForward * m_CameraMovementSpeed;
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
		m_CameraPos -= m_CameraRight * m_CameraMovementSpeed;
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
		m_CameraPos += m_CameraRight * m_CameraMovementSpeed;

	if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS))
		m_CameraPos -= m_CameraUp * m_CameraMovementSpeed * 10.f;
	if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS))
		m_CameraPos += m_CameraUp * m_CameraMovementSpeed * 10.f;
}

void VulkanBase::mouseMove(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec2 curMousePos = glm::vec2(xpos, ypos);
	auto deltaMousePos = curMousePos - m_PrevMousePos;
	m_PrevMousePos = curMousePos;

	int leftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (leftState == GLFW_PRESS)
	{
		m_Yaw += deltaMousePos.x * m_MouseSensitivity;
		m_Pitch += deltaMousePos.y * m_MouseSensitivity;

		m_Yaw = fmod(m_Yaw, glm::radians(89.f));
		m_Pitch = fmod(m_Pitch, glm::radians(89.f));
	}
}
void VulkanBase::mouseEvent(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		std::cout << "left mouse button pressed\n";
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos); 
		m_PrevMousePos.x = static_cast<float>(xpos);
		m_PrevMousePos.y = static_cast<float>(ypos);
	}
}

glm::mat4 VulkanBase::UpdateCamera()
{
	glm::vec3 newForward{ glm::normalize(glm::rotate(glm::mat4(1.0f), -m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
					  glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
					  glm::vec4(m_CameraForward, 1.0f))
	};

	m_CameraForward = newForward;
	m_CameraRight = glm::cross(m_CameraForward, glm::vec3(0.f, 1.f, 0.f));
	m_CameraUp = glm::cross(m_CameraRight, m_CameraForward);

	glm::mat4 viewMatrix{ glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp) };

	return viewMatrix;
}