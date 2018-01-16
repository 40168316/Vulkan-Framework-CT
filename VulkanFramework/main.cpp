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
#include <fstream>

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
	// Vector which is used to store the framebuffers and the attachments - is a collection of buffers that can be used as the destination for rendering (attachtment of swap chain colour attactment)
	std::vector<VkFramebuffer> swapChainFramebuffers;
	// Vector which stores the information regarding the swap chain image views - creates a basic image view for every image in the swap chain
	std::vector<VkImageView> swapChainImageViews;
	// Member variable which stores the pipeline state - stores different uniform values which can be changed at drawing time to alter the behaviour of shaders without recreation
	VkPipelineLayout pipelineLayout;
	// Member variable which stores the render pass - uses the colour attachtments and supasses to create a pass 
	VkRenderPass renderPass;
	// Member variable which stores thge graphics pipeline
	VkPipeline graphicsPipeline;
	// Member variable which manage tge memory that is used to store the buffers and command buffers are allocated from them
	VkCommandPool commandPool;
	// Vector of command buffers that are executed by submitting them on one of the device queues
	std::vector<VkCommandBuffer> commandBuffers;
	// Semaphores synchronise operations within or across command queues - check if the image is ready for rendering and signal that rendering has finished. 
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

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
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSemaphores();
	}

	// Function which creates the required semaphores - synchronise operations within or across command queues - check if the image is ready for rendering and signal that rendering has finished. 
	void createSemaphores() 
	{
		// Struct which specifies the semaphore nfo/type
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Initiate the two semaphores - if not successful show an error
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}

	// Function which stores additinal pipeline information such as the framebuffer attachtments - ie how many colour and dpeth buffers there will be 
	void createRenderPass() 
	{
		// Struct called colour attachment which details all the framebuffer attachments for the graphics pipeline 
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;  // ormat of the color attachment should match the format of the swap chain images as there is only one 
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // As there is nothing happening with multisampling set the value to one 
		// The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering
		// The loadOp and storeOp apply to color and depth data
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the values to a constant at the start 
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Rendered contents will be stored in memory and can be read later
		// stencilLoadOp / stencilStoreOp apply to stencil data
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Existing contents are undefined; we don't care about them
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Contents of the framebuffer will be undefined after the rendering operation
		// Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format
		// The initialLayout specifies which layout the image will have before the render pass begins.
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// The finalLayout specifies the layout to automatically transition to when the render pass finishes
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Struct colorAttachmentRef which details the colour attachment type 
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Gives the best performance 

		// A single render pass can consist of multiple subpasses. 
		// Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes. 
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		// Include the colour attachment struct above 
		subpass.pColorAttachments = &colorAttachmentRef;

		// Struct which stores information about subpass dependecies - where a check is made to make sure the image is avaiable for the render pass
		VkSubpassDependency dependency = {};
		// Specify the indices of the dependency and the depend subpass 
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // External refers to the implicit subpass before or after the render pass
		dependency.dstSubpass = 0;
		// Specify the operations to wait on and the stages in which these operations occur - need to wait for the swap chain to finish reading the image before it can be accessed
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
		dependency.srcAccessMask = 0;
		// The operations that should wait on this are in the color attachment stage and involve the reading and writing of the color attachment
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Render pass struct which details the render pass information and includes other structs 
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		// Include the color attachment struct 
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		// Include the sub pass struct 
		renderPassInfo.pSubpasses = &subpass;
		// Connect the render pass to the depenency struct above
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		// Create the render pass - if not successful throw an error 
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}
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
			drawFrame();
		}

		// Wait for logical device to finish before destorying 
		vkDeviceWaitIdle(device);
	}

	// Method which deals with acquiring an image from the swap chain, execute the command buffer and returns the image to the swap chain for presentation
	void drawFrame() 
	{
		uint32_t imageIndex;
		// Acquire the next image from the swap chain using the logical device, swaphcain, timeout in nanoseconds, the semaphore, handle and reference to image index
		vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		// Struct which is used for queue submission and synchronization is configured through parameters
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// Semaphore information part of submit info struct
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore }; // Wait onbefore execution begins 
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // Write the colours to the attachment
		submitInfo.waitSemaphoreCount = 1; 
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Specify which command buffers to actually submit for execution
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		// Specify which semaphores to signal once the command buffers have finished execuion
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Submit the command buffer to the graphics queue - if not successful throw an error 
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// Struct which deals with submitting the results back to the swap chain to have it eventually show up on the screen 
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// Specify which semaphores to wait on before presentation can happen
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		// Specify the swap chains to present images to and the index of the image for each swap chain
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		// Sumbits the request to present an image to the swap chain 
		vkQueuePresentKHR(presentQueue, &presentInfo);

		// Deal with the memory leak - this is due to the validation layers expecting the application to sync with the GPU which doesnt always happen 
		vkQueueWaitIdle(presentQueue);
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
		// Destroy the semaphore
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		// Destroy the commandpool
		vkDestroyCommandPool(device, commandPool, nullptr);
		// Destory the framebuffer 
		for (auto framebuffer : swapChainFramebuffers) 
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		// Destroy the graphics pipeline 
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// Destroy the pipeline
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// Destory the render pass 
		vkDestroyRenderPass(device, renderPass, nullptr);
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

	// Function which reads in the shaders and puts them into the graphics pipeline
	static std::vector<char> readFile(const std::string& filename) 
	{
		// Get the file name with two flags, ate = start reading at the end of the file, binary = read the file as a binary file 
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		// If file cannot be opened then throw an error 
		if (!file.is_open()) 
		{
			throw std::runtime_error("failed to open file!");
		}

		// By reading from the end of the file with the ate flag (ABOVE), the size can be determined to allocate a buffer
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		//Return to the start of the file and read all the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		// Close file
		file.close();
		// Return the bytes in the buffer
		return buffer;
	}

	// Method which creates the graphics pipeline
	void createGraphicsPipeline() 
	{
		// Read the vertex and fragment shaders into the applicaiton 
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		// Vertex and fragment shader modules which wraps the shader code into a shader module 
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// Create a shader stage which links the shaders to each other and give them a purpose
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule; // Specify the mode to the shader - the link 
		vertShaderStageInfo.pName = "main"; 

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule; // Link to the fragment module 
		fragShaderStageInfo.pName = "main";

		// A struct which contains both the vertex and fragment shader stage structs 
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex Input - function descibes the format of the vertex data - bindings = spacing between the data and if data is per pixel or per instance 
		// Attribute description - type of attribute passed to the vertex shader which binding to load them from and the offset 
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		// Input assemby struct which describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle from every 3 vertices without reuse
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// A viewport basically describes the region of the framebuffer that the output will be rendered to
		VkViewport viewport = {};
		viewport.x = 0.0f; // From 0,
		viewport.y = 0.0f; // 0 
		viewport.width = (float)swapChainExtent.width; // To width,
		viewport.height = (float)swapChainExtent.height; // Height - ie fullscreen 
		viewport.minDepth = 0.0f; // Lowest possible value
		viewport.maxDepth = 1.0f; // Highest possible value 

		// No scissoring so specify a rectangle that covers the framebuffer entriely
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		// Viewport State = combined viewport (region of framebuffer reneder too) and scissor rectangle 
		// Struct which creates a viewport state by referncing the viewports and scissors - counts are at one as some gfx cards support multiple viewports at once 
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them
		rasterizer.depthClampEnable = VK_FALSE;
		// If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		// The polygonMode determines how fragments are generated for geometry. 
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // fill the area of the polygon with fragments
		//rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // polygon edges are drawn as lines
		//rasterizer.polygonMode = VK_POLYGON_MODE_POINT; // polygon vertices are drawn as points
		// The lineWidth member is straightforward, it describes the thickness of lines in terms of number of fragments.
		rasterizer.lineWidth = 1.0f;
		// The cullMode variable determines the type of face culling to use
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Currently cullling the back face 
		// The frontFace variable specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// Depth values is not used so set to false 
		rasterizer.depthBiasEnable = VK_FALSE;

		// Multisampling which is one of the ways to perform anti-aliasing
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE; // Set to false as multisampling is not used 
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Colour Blending is after the fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
		// This struct contains the configuration per attached framebuffer
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		// This struct references the array of structures for all of the framebuffers and 
		// allows you to set blend constants that you can use as blend factors in the aforementioned calculations.
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Pipeline Layout - stores different uniform values which can be changed at drawing time to alter the behaviour of shaders without recreation
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		// Initiate the pipeline layout using the struct above - if not successful throw error 
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// Struct which pulls all the above structs together to make the graphics pipeline 
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Initiate the graphics pipeline - if not successful throw an error 
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// Destroy both the vertex and shader modules when the pipeline is exited 
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	// Shader code needs to be wrapped in a VKShaderModule object before being passed to the pipeline - function
	VkShaderModule createShaderModule(const std::vector<char>& code) 
	{
		// Struct which stores information  specifying a pointer to the buffer with the bytecode and the length of it
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// Create the shader module
		VkShaderModule shaderModule;
		// If the shader module is not successful in initiation then throw an error 
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create shader module!");
		}

		// Return complete shader module 
		return shaderModule;
	}

	// Methid which stores the framebuffers and attachments - is a collection of buffers that can be used as the destination for rendering (attachtment of swap chain colour attactment)
	void createFramebuffers() 
	{
		// Resize the container to hold all of the framebuffers
		swapChainFramebuffers.resize(swapChainImageViews.size());

		// Iterate through the image views and create framebuffers from them
		for (size_t i = 0; i < swapChainImageViews.size(); i++) 
		{
			VkImageView attachments[] = 
			{
				swapChainImageViews[i]
			};

			// Create a struct which stores the info of the framebuffer
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// Specify the render pass the framebuffer requires to be compatible 
			framebufferInfo.renderPass = renderPass;
			// Set attachment count as one as there is only the coulour attachment
			framebufferInfo.attachmentCount = 1;
			// Specify the attachments the framebuffer requires to be compatible
			framebufferInfo.pAttachments = attachments;
			// Set the width and height based on the swap chain width and height
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			// Set to one - refers to the number of layers in image arrays
			framebufferInfo.layers = 1;

			// Initialise the framebuffer - if not successful throw an error 
			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	// Function which manage the memory that is used to store the buffers and command buffers are allocated from them
	void createCommandPool() 
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		// Create a struct which stores the command pool information
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Select a graphics family queue for drawing commands 
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		// Initalise the command pool - if not successful then throw error 
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	// Function which creates command buffers which stores all the operation you want to perform 
	void createCommandBuffers() 
	{
		// Resuze the command buffer based on the size of the swapchain buffer
		commandBuffers.resize(swapChainFramebuffers.size());

		// Struct which specifies the command pool and the number of buffers to allocate
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		// Primary instead of secondary - can be submitted to a queue for execution, but cannot be called from other command buffers.
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Level parameter specifies if the allocated command buffers are primary or secondary command buffers.
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		// Initiate the allocate command buffers - if not successful then throw an error 
		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		// Begin recording the command buffer
		for (size_t i = 0; i < commandBuffers.size(); i++) 
		{
			// Struct that specifies some details about the usage of this specific command buffer.
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT - The command buffer can be resubmitted while it is also already pending execution.
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Flags parameter specifies how we're going to use the command buffer

			// Initiate and begin the command buffer
			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			// To draw, start by creating a render pass 
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			// Set the render pass to the preset render pass
			renderPassInfo.renderPass = renderPass;
			// Create a framebuffer for each swap chain image that specifies it as colour attachment 
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			// Define the size of the render area - this defines where shader loads and stores will take place 
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			// Struct which defines the load operation for the colour attachment, defined as black - MIGHT BE BACKGROUND COLOUR
			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			// Begin the render pass using the struct created above 
			// Command buffer to record the command to, render pass struct, controls how the drawing commands within the render pass will be provided - INLINE
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind the graphics pipeline 
			// Command buffer to record the command to, pipeline object is a graphics pipeline, 
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			// Draw the command buffers (vertex count, instanceCount, firstVertex, firstInstance)
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			// End the render pass 
			vkCmdEndRenderPass(commandBuffers[i]);

			// Finish recording the command buffer - if not successful throw error 
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to record command buffer!");
			}
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