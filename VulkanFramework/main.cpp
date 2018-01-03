#define GLFW_INCLUDE_VULKAN

// Include the Vulkan SDK giving access to functions, structures and enumerations
#include <vulkan/vulkan.h>

// Include the GLFW header required for the window 
#include <GLFW\glfw3.h>

// Include headers used for the application
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>

// Width and Heigh of the window - used throught the application 
const int WIDTH = 800;
const int HEIGHT = 600;

// Enable a range of useful diagnostics layers instead of spefic ones 
const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_LUNARG_standard_validation"
};

// If not in debug mode then set the validation layers to false - else to true - allows for proformance improve in release with less overheads
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Class which contains the Vulkan Objects
class VulkanObjects 
{
	// Public modifier - can be seen by all
public:
	// Run method which contains all the private class members 
	void run() 
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
	
	// Private modifier - only class has access 
private:
	// Member variable which stores a reference to the window
	GLFWwindow* window;
	// Member variable which stores the instance (connection between application and Vulkan Library) - handle to the instance 
	VkInstance instance;

	// Method which initiates various Vulkan calls 
	void initVulkan() 
	{
		createInstance();
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
		// Destory the instance between the Vulkan Library and application just before the application closes
		vkDestroyInstance(instance, nullptr);
		// Destroy the window
		glfwDestroyWindow(window);
		// Terminate access to GLFW
		glfwTerminate();
	}

	// Method which creates the applications instance (link between the Vulkan Library and the application)
	void createInstance() 
	{
		// Struct with some information about the application - optional but useful to optimise the drivers. 
		// Define the struct as appInfo
		VkApplicationInfo appInfo = {};
		// Information below sets is about the application
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Struct which tells the Vulkan Driver which global extensions and validations layers are desired - not optional 
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Create a GLFW extension 
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		// Get the required GLFW extensions for the instance 
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// Get two desired global extensions - this allows for interfacing with the window system 
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		// Set layer count to zero
		createInfo.enabledLayerCount = 0;

		// As everything is now setup for an instance, an instance can now be created using the struct - catch which produces an error code if failed
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create instance!");
		}
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