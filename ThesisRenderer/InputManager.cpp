#include "InputManager.h"

GLFWwindow* InputManager::window = nullptr;

void InputManager::Init(GLFWwindow* win)
{
    window = win;
}

bool InputManager::IsKeyPressed(int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputManager::IsMouseButtonPressed(int button)
{
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void InputManager::GetMousePosition(double& x, double& y)
{
    glfwGetCursorPos(window, &x, &y);
}