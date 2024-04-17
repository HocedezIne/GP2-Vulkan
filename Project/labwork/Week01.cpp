#include "vulkanbase/VulkanBase.h"

void VulkanBase::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanBase::keyEvent(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos += m_CameraForward * 0.1f;
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos -= m_CameraForward * 0.1f;
	}
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos -= m_CameraRight *.1f;
	}
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPos += m_CameraRight  * 0.1f;
	}
}

void VulkanBase::mouseMove(GLFWwindow* window, double xpos, double ypos)
{
	int leftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (leftState == GLFW_PRESS)
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
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		std::cout << "left mouse button pressed\n";
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		m_DragStart.x = static_cast<float>(xpos);
		m_DragStart.y = static_cast<float>(ypos);
	}
}