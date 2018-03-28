#include "WindowManager.h"
#include "FrameworkSingleton.h" // Gives access to singleton and required libraries

WindowManager::WindowManager()
{
}


WindowManager::~WindowManager()
{
}

// Method which initialises the GLFW window
void WindowManager::initWindow()
{
	// Initialise the GLFW Library
	glfwInit();
	// Setup for Vulkan - set API to no as GLFW was designed for OpenGL with this application being using Vulkan instead
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// Disable resizable window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	// Create the window using the member variable - width, height, title, viewing monitor, OpenGL only.
	FrameworkSingleton::getInstance()->window = glfwCreateWindow(FrameworkSingleton::getInstance()->WIDTH, FrameworkSingleton::getInstance()->HEIGHT, "Vulkan", nullptr, nullptr);
}