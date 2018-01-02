#define GLFW_INCLUDE_VULKAN

// Include the Vulkan SDK giving access to functions, structures and enumerations
#include <vulkan/vulkan.h>

// Include the GLFW header required for the window 
#include <GLFW\glfw3.h>

// Include headers used for the application
#include <iostream>
#include <stdexcept>
#include <functional>

// Width and Heigh of the window - used throught the application 
const int WIDTH = 800;
const int HEIGHT = 600;

// Class which contains the Vulkan Objects
class VulkanObjects 
{
public:
	// Run method which contains all the private class members 
	void run() 
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	// Member variable which stores a reference to the window
	GLFWwindow* window;
	// Method which initiates various Vulkan calls 
	void initVulkan() 
	{

	}

	// Method which loops through until the window closes
	void mainLoop() 
	{
		// While window is not closed - Checks for end commands such as the X button 
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	// Method which initialises the GLFW window
	void initWindow() 
	{
		// Initialise the GLFW Library
		glfwInit();
		// Setup for Vulkan - set API to no as GLFW was designed for OpenGL with this application being using Vulkan instead
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Disable resizable window
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		// Create the window using the member variable - width, height, title, viewing monitor, OpenGL only.
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	// Once the window is closed, deallocation of resources occurs 
	void cleanup() 
	{
		// Destroy the window
		glfwDestroyWindow(window);
		// Terminate access to GLFW
		glfwTerminate();
	}
};

// Main method
int main() {
	// Class VulkanObjects
	VulkanObjects app;

	// Try to run Vulkan Objects
	try {
		app.run();
	}
	// If app.run fails then catch with a runtime error 
	catch (const std::runtime_error& e) {
		// If error, display what the error is and exit as failure
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}