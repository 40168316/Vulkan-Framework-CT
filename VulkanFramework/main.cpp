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
#include <set>
#include <algorithm> 

// Width and Heigh of the window - used throught the application 
const int WIDTH = 800;
const int HEIGHT = 600;

// Enable a range of useful diagnostics layers instead of spefic ones 
const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_LUNARG_standard_validation"
};

// VK_KHR_swapchain device extension which enables the swap chain via extension - similar to validation layers 
const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

// Struct which checks queue families supporting drawing commands and the ones supporting presentation do not overlap
struct QueueFamilyIndices 
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

// A struct that returns the indices of the queue families that satisfy certain desired properties
struct QueueFamilyIndices 
{
	int graphicsFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0;
	}
};

// Struct which stores details of swap chain support - query the physical device for some details 
struct SwapChainSupportDetails 
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
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
	// Member variable which is a queue which deals specifically with graphics
	VkQueue graphicsQueue;
	// Member variable which is used to establish the connection between Vulkan and the window system to present results to the screen
	VkSurfaceKHR surface;
	// Member variable which is a queue which deals specifically with presentation
	VkQueue presentQueue;
	// Member variable which contains the swap chain and all the parts that are included in it
	VkSwapchainKHR swapChain;
	// Memeber vector which stores the swap chain handles 
	std::vector<VkImage> swapChainImages;
	// Member variables which store the format and extent chosen for the swap chain images 
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	// Vector which stores the information regarding the swap chain image views - creates a basic image view for every image in the swap chain
	std::vector<VkImageView> swapChainImageViews;

	// Method which initiates various Vulkan calls 
	void initVulkan() 
	{
		createInstance();
		setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
	}

	// Function which creates image views - creates a basic image view for every image in the swap chain
	void createImageViews() 
	{
		// Resize the list to fit all of the image views we'll be creating
		swapChainImageViews.resize(swapChainImages.size());

		// Loop which iterates over all the swap chain images 
		for (size_t i = 0; i < swapChainImages.size(); i++) 
		{
			// Create a struct which stores information about the images views for the swap chain
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			// The viewType and format fields specify how the image data should be interpreted
			// The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			// The components field allows you to swizzle the color channels around
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//The subresourceRange field describes what the image's purpose is and which part of the image should be accessed
			// Our images will be used as color targets without any mipmapping levels or multiple layers.
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// Create the image view by using the information specified above, the logical device and the swap chain image views - if not successful throw an error 
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	// Create the swapchain by bring together the surface format, present mode and extent together.
	void createSwapChain() 
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		// Add the different required parts to the swap chain 
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Count the number of images in the swap chain - value of 0 for maxImageCount means that there is no limit besides memory requirements hence a check is required
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Create a struct which holds details of the swap chain 
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		// Specify the image information 
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Specify how the swap chain will be handled if queue types are different
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

		// If the indices queues are different then make the image concurrent meaning it can be showed across multiple family queues 
		if (indices.graphicsFamily != indices.presentFamily) 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		// else if the indice queries are the same then make the image exclusive meaning it can only be part of one queue family at one time 
		else 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// Specify the tyoe of transform to be applied to image - ie 90 degrees left or flip horizontally
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		// Apla channel which deals with transparency
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// If the clipped member is set to VK_TRUE then that means that we don't care about the color of pixels that are obscured - ie another window is in front of them
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		// With Vulkan it's possible that your swap chain becomes invalid or unoptimized while your application is running, for example because the window was resized
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Initiate the swap chain and if it was not successful then throw an error 
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		// Get the images required for the swap chain 
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		// Store the format and extent of the swap chain 
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	// Create a surface which deals with the connection between Vulkan and the window system to present results to the screen
	void createSurface() 
	{
		// Initiate the window surface - if not successful throw an error 
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Function which creates a logical device - used to inferface with the logical device - used to specify which queues we want to use from the ones available. 
	void createLogicalDevice() 
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// Create a vector which will store information about the different queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		// Added two queue families, one for graphics and the second for presentation
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) 
		{
			// Struct which defines the number of queues we want for a single queue family
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // Define the type of which is queue
			queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; // This queue is regarding graphics capabilities 
			queueCreateInfo.queueCount = 1; // Increase queue count to 1
			queueCreateInfo.pQueuePriorities = &queuePriority; // Set priority
			queueCreateInfos.push_back(queueCreateInfo); // Add to vector
		}

		// Used for specifying device features 
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Creation of the logical device 
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		// Add a pointer to the queue created above and increase the count
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		// Specify the logical device and enable the features
		createInfo.pEnabledFeatures = &deviceFeatures;

		// Counter for enabling the validation layers but this time for the physical device 
		createInfo.enabledExtensionCount = 0;

		// Enable the extensions required for the swap chain 
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// If Vlaidation Layers are enabled then
		if (enableValidationLayers) 
		{
			// Count and add the data
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		// Else set the counter to zero 
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Initiate the logical device - if it does not return a VK_success then throw error 
		// Parameters (physical device selected, logical device info, .., and the logical device itslef
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		// Retrieving queue handles 
		// Parameters (the logical device, queue family, queue index and a pointer to the variable to store the queue handle in)
		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
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

	// Function which contains details to query the physical device to see if it can support a swap chain 
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) 
	{
		SwapChainSupportDetails details;

		// Check the physical device for surface capabilities 
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Create a format count to count the supported surface formats avialable 
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		// If format count is not equal to zero then
		if (formatCount != 0) 
		{
			// Resize the vector to be able to store the additional formats 
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// Query the support of the presentation mode - exactly the same as above accept with presentation 
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) 
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	// Go through the found physical devices to check if they support Vulkan 
	bool isDeviceSuitable(VkPhysicalDevice device) 
	{
		// Check if the necessary queue families are supported. 
		QueueFamilyIndices indices = findQueueFamilies(device);

		// Check is extension suuport for swap chain exists for the physical device 
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// Check the device to make sure that swap chain and its requirements are supported. 
		bool swapChainAdequate = false;
		if (extensionsSupported) 
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	// 
	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		// Create an extension count to allow for mulitple extensions to be checked for 
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		// Create a vector of available extensions
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Create a set which goes through all the required extensions 
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) 
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
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
		// Destroy the imagesviews that are part of the swap chain 
		for (auto imageView : swapChainImageViews) 
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
		// Destory the swap chain 
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		// Destroy the logical device 
		vkDestroyDevice(device, nullptr);
		// Destroy the debug report call back which relys any error messages through the use of validation layers 
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
		// Destroy the Vulkan surface object
		vkDestroySurfaceKHR(instance, surface, nullptr);
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
		for (const auto& queueFamily : queueFamilies) 
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.graphicsFamily = i;
			}

			// Look for a queue family that has the capability of presenting to our window surface
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			// Check the value of the boolean and store the presentation family queue index
			if (queueFamily.queueCount > 0 && presentSupport) 
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete()) 
			{
				break;
			}

			i++;
		}

		return indices;
	}

	// Function which chooses the surface format - color depth/ color channels and types used in the application such as the RGB colour system
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// If a surface format is avialable and but is not defined then set the surface format to VK_FORMAT_B8G8R8A8_UNORM which is RGB om 8bit unsigned intergers (in short)
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		// If the surface format is not available then go through the list and see if the presefered combination is avaialable 
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	// Function which chooses the guaranteed available presentation mode for the swap chain 
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) 
	{
		// Set the best mode as default - ganuenteed to be supported by all physical devices that have made it this far 
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		// Loop through all te available modes to see if mailbox with triple box is avialable 
		for (const auto& availablePresentMode : availablePresentModes) 
		{
			// If certain swap modes are available then apply the swap chain 
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
			{

				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	// Function which chooses the swap extent - the resolution of the swap chain images 
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			VkExtent2D actualExtent = { WIDTH, HEIGHT };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void createGraphicsPipeline() 
	{

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