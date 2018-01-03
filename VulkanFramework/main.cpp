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

// Proxy function which passes the vkCreateDebugReportCallbackExt to create the VkDebugReportCallbackEXT object. As nested functions cannot exist, this proxy function acts as bridge to automatically load the debug report. 
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) 
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// Proxy function which destorys the VkDebugReportCallbackEXT which is created in the above function
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	// If the func - VkDebugReportCallbackEXT - is not a null pointer then attempt to destory again
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

// A struct that returns the indices of the queue families that satisfy certain desired properties
struct QueueFamilyIndices 
{
	int graphicsFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0;
	}
};

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
	// Member variable which deals with the debug call back - relying of the error message if an error occurs using validation layers
	VkDebugReportCallbackEXT callback;
	// Selected graphics card based on the pickPhysicalDevice method will be stored in this member variable
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	// Member variable which stores the logical device - built upon the physical device and is used to interface with it - works as a bridge 
	VkDevice device;

	// Method which initiates various Vulkan calls 
	void initVulkan() 
	{
		createInstance();
		setupDebugCallback();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	// Function which creates a logical device - used to inferface with the logical device - used to specify which queues we want to use from the ones available. 
	void createLogicalDevice() 
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		// Struct which defines the number of queues we want for a single queue family 
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; // This queue is regarding graphics capabilities 
		queueCreateInfo.queueCount = 1;
	}

	// Function that picks a physical device based on the support of features required
	void pickPhysicalDevice() 
	{
		// Counter for the number of graphics cards
		uint32_t deviceCount = 0;
		// Find the number of physical devices in the machine/Vulkan has access too
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If no physical devices can support Vulkan then throw an error as application cannot continue running 
		if (deviceCount == 0) 
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Array which stores all of the VkPhysicalDevice handles 
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// For all physical devices found 
		for (const auto& device : devices) 
		{
			// If decive is suitiable is true - calls function below
			if (isDeviceSuitable(device)) 
			{
				// Make physical device - the device we will use - to the current device. This application uses the first suitable physical device found
				physicalDevice = device;
				break;
			}
		}

		// If physical device has no device assigned to it after going through the devices then throw error 
		if (physicalDevice == VK_NULL_HANDLE) 
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	// Go through the found physical devices to check if they support Vulkan 
	bool isDeviceSuitable(VkPhysicalDevice device) 
	{
		// Check if the necessary queue families are supported. 
		QueueFamilyIndices indices = findQueueFamilies(device);

		return indices.isComplete();
	}

	// Function which sets up the debug call back - relying of the error message if an error occurs using validation layers
	void setupDebugCallback() 
	{
		// If validation layers are disabled or not available then return 
		if (!enableValidationLayers) return;
		// Create a struct which details useful information about the call back 
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT; // Define the type of callback 
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT; // Flags field allows you to filter which types of messages you would like to receive
		createInfo.pfnCallback = debugCallback; // pfnCallback field specifies the pointer to the callback function

		// If the debug call back report does not exist then error
		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to set up debug callback!");
		}
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
		// Destroy the debug report call back which relys any error messages through the use of validation layers 
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
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
		// If validations layers are enabled and no validation support exists then throw error
		if (enableValidationLayers && !checkValidationLayerSupport()) 
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

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

		// Get the required extensions that are needed for call backs to rely an error message if an error occurs 
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// If validation layers are enabled 
		if (enableValidationLayers) 
		{
			// Get the validation layer count
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			// Get the validation layer names
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		// Else if the validation layers are disabled or not available then set the count to zero 
		else 
		{
			createInfo.enabledLayerCount = 0;
		}

		// As everything is now setup for an instance, an instance can now be created using the struct - catch which produces an error code if failed
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	// Function that checks if all of the requested layers are available 
	bool checkValidationLayerSupport() 
	{
		// Create a counter for available layers 
		uint32_t layerCount;
		// Below lists all the available extensions and counts them up
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Create a vector which stores the available layers
		std::vector<VkLayerProperties> availableLayers(layerCount);
		// Add the data 
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Create a for loop which will check if all of the layers in validationLayers exist in the availableLayers list
		for (const char* layerName : validationLayers) 
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) 
			{
				return false;
			}
		}

		return true;
	}

	// Function which gets the required extension for the message callback to rely to error 
	// More specifically - this is a function that will return the required list of extensions based on whether validation layers are enabled or not
	std::vector<const char*> getRequiredExtensions() {
		// Create a vector of extensions
		std::vector<const char*> extensions;

		// GLFW extensions are always required 
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (uint32_t i = 0; i < glfwExtensionCount; i++) 
		{
			extensions.push_back(glfwExtensions[i]);
		}

		// If the validation layer is avaiable then add to the vector 
		if (enableValidationLayers) 
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	// Static function which called debugCallBack which deals with the debug messages 
	// The VKAPI_ATTR and VKAPI_CALL ensure that the function has the right signature for Vulkan to call it
	// Parameters (type of message - can be a combination of any flags, type of object subject to the error, obj is the object - ie VkPhysicalDevice, .., .., .., msg parameter contains the pointer to the message itself, userData parameter to pass your own data to the callback)
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) 
	{
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	// Function which finds the different types of queue families - note is takes in the device to check if the device supports these queues 
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		// Set a counter for the amount of different types of queue families 
		uint32_t queueFamilyCount = 0;
		// Count the number of types of queue families 
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		// Get the family queues data 
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// Search to find atleast one queue family that supports Vk_QUEUE_GRAPHICS_BIT
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) 
			{
				break;
			}

			i++;
		}

		return indices;
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