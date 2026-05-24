#pragma once

#include <GLFW/glfw3.h>

class InputManager
{
public:

    static void Init(GLFWwindow* window);

    static bool IsKeyPressed(int key);

    static bool IsMouseButtonPressed(int button);

    static void GetMousePosition(double& x, double& y);

private:
    static GLFWwindow* window;
};