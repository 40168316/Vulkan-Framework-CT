#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Include the Vulkan SDK giving access to functions, structures and enumerations
#include <vulkan/vulkan.h>

// Include the GLFW header required for the window 
#include <GLFW\glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include image loader library 
#define STB_IMAGE_IMPLEMENTATION

// Include OBJ loader library
#define TINYOBJLOADER_IMPLEMENTATION

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Include headers used for the application
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <set>
#include <algorithm> 
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>

// Include other header files
#include "camera.h"
#include "free_camera.h"
#include "target_camera.h"
#include "WindowManager.h"
#include "CameraManager.h"
#include "CleanUpManager.h"
#include "VulkanManager.h"
#include "SceneManager.h"

struct Vertex;
struct SwapChainSupportDetails;
struct QueueFamilyIndices;
struct UniformBufferObject;

extern const std::vector<const char*> deviceExtensions;
extern const std::vector<const char*> validationLayers;

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

// If not in debug mode then set the validation layers to false - else to true - allows for proformance improve in release with less overheads
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

extern const std::vector<Vertex> cubeVertices1;
extern const std::vector<Vertex> cubeVertices2;
extern const std::vector<Vertex> cubeVertices3;
extern const std::vector<Vertex> skyboxVertices;
extern const std::vector<uint32_t> planeIndices;
extern const std::vector<uint32_t> cubeIndices;
extern const std::vector<uint32_t> skyboxIndices;



class FrameworkSingleton
{
public:
	FrameworkSingleton();
	~FrameworkSingleton();

	AllCamera::free_camera* freeCam;
	// Create vector to apply to current cam pos - just an initial postion
	glm::vec3 freeCamPos = glm::vec3(0.0f, 0.0f, 0.0f);
	double cursor_x = 0.0, cursor_y = 0.0;
	float cameraSpeed = 0.001f;

	AllCamera::target_camera* targetCamera;

	// Width and Heigh of the window - used throught the application 
	const int WIDTH = 800;
	const int HEIGHT = 600;

	// Set model paths
	const std::string modelSceneryPath = "models/mountains.obj"; // Scenery
	const std::string modelChaletPath = "models/chalet.obj"; // Chalet

	// Set texture paths
	const std::string boxesTexturePath = "textures/box.jpg"; // Boxes
	const std::string checkedTexturePath = "textures/checks.jpg"; // Checkered board
	const std::string modelSceneryTexturePath = "textures/terrain3.jpg"; // Scenery
	const std::string modelChaletTexturePath = "textures/chalet.jpg"; // Chalet

	// Skybox textures
	const std::string topSkyTexturePath = "textures/skyboxes/top.png";
	const std::string bottomSkyTexturePath = "textures/skyboxes/bot.png";
	const std::string leftSkyTexturePath = "textures/skyboxes/left.png";
	const std::string rightSkyTexturePath = "textures/skyboxes/right.png";
	const std::string frontSkyTexturePath = "textures/skyboxes/front.png";
	const std::string backSkyTexturePath = "textures/skyboxes/back.png";

	int cameraType = 0;

	int NUMBEROFSHAPES = 6;

	VkImageViewType twoDImageView = VK_IMAGE_VIEW_TYPE_2D;
	VkImageViewType cubeImageView = VK_IMAGE_VIEW_TYPE_CUBE;

	WindowManager winManager;
	CameraManager camManager;
	VulkanManager vulkanManager;
	CleanUpManager cleanUpManager;
	SceneManager sceneManager;

	// Run method which contains all the private class members 
	void run()
	{
		// Setup output file
		std::ofstream data("data.csv", std::ofstream::out);
		// Record the start time 
		auto start = std::chrono::system_clock::now();

		// Init window, Vulkan and cameras
		winManager.initWindow();
		vulkanManager.initVulkan();
		camManager.initCameras();
		// While window is not closed - loop 
		while (!glfwWindowShouldClose(FrameworkSingleton::getInstance()->window))
		{
			sceneManager.input();
			sceneManager.update();
			// Record the end time
			auto end = std::chrono::system_clock::now();
			// Get the total time by taking the end time away from the start time
			auto total = end - start;
			// Output to a file
			data << std::chrono::duration_cast<std::chrono::milliseconds>(total).count() << std::endl;
		}
		// Wait for logical device to finish before destorying 
		vkDeviceWaitIdle(FrameworkSingleton::getInstance()->device);
		cleanUpManager.cleanup();
	}

	// Static singleton access method
	static FrameworkSingleton* getInstance();

	// Store singleton instance
	static FrameworkSingleton* singletonInstance;
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
	VkPipeline skyboxGraphicsPipeline;
	// Member variable which manage tge memory that is used to store the buffers and command buffers are allocated from them
	VkCommandPool commandPool;
	// Vector of command buffers that are executed by submitting them on one of the device queues
	std::vector<VkCommandBuffer> commandBuffers;
	// Semaphores synchronise operations within or across command queues - check if the image is ready for rendering and signal that rendering has finished. 
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	// Vectors which contain the vertices and indices for the model
	std::vector<Vertex> modelChaletVertices;
	std::vector<uint32_t> modelChaletIndices;
	std::vector<Vertex> modelSceneryVertices;
	std::vector<uint32_t> modelSceneryIndices;
	// Vertex Buffer object 
	VkBuffer vertexBox1, vertexBox2, vertexBox3;
	VkBuffer vertexChaletModel;
	VkBuffer vertexSceneryModel;
	VkBuffer vertexSkybox;
	// Vertex Buffer memory object which holds the memory regarding the vertex buffer 
	VkDeviceMemory vertexBox1Memory, vertexBox2Memory, vertexBox3Memory;
	VkDeviceMemory vertexChaletModelMemory;
	VkDeviceMemory vertexSceneryModelMemory;
	VkDeviceMemory vertexSkyboxMemory;
	// Index buffer object
	VkBuffer indexBox;
	VkBuffer indexPlane;
	VkBuffer indexChaletModel;
	VkBuffer indexSceneryModel;
	VkBuffer indexSkybox;
	// Vertex Buffer memory object which holds the memory regarding the vertex buffer 
	VkDeviceMemory indexBoxMemory;
	VkDeviceMemory indexPlaneMemory;
	VkDeviceMemory indexChaletModelMemory;
	VkDeviceMemory indexSceneryModelMemory;
	VkDeviceMemory indexSkyboxMemory;
	// Descriptor layout used for specifying the layout for the uniform buffers
	VkDescriptorSetLayout descriptorSetLayout;
	// Uniform buffer object which is used to store the uniform buffer
	VkBuffer uniformBuffer;
	VkBuffer rotatingUniformBuffer;
	// Uniform buffer object memory 
	VkDeviceMemory uniformBufferMemory;
	VkDeviceMemory rotatingUniformBufferMemory;
	// Descriptor pool object which is used to get descriptor sets
	VkDescriptorPool descriptorPool;
	// Descriptor set which is gets sets from the pool
	VkDescriptorSet cubedescriptorSet;
	VkDescriptorSet checkedDescriptorSet;
	VkDescriptorSet modelSceneryDescriptorSet;
	VkDescriptorSet modelChaletDescriptorSet;
	VkDescriptorSet skyboxDescriptorSet;
	// VkImage objects which hold images information
	VkImage boxesTexture; // Boxes
	VkImage modelChaletTexture; // Chalet
	VkImage modelSceneryTexture; // Scenery
	VkImage checkedTexture; // Checked
	VkImage frontSkyTexture, backSkyTexture, leftSkyTexture, rightSkyTexture, topSkyTexture, bottomSkyTexture; // Skybox
	// Texture image memory 
	// VkDevice Memory used to store the images data
	VkDeviceMemory boxesTextureMemory; // Boxes
	VkDeviceMemory modelChaletTextureMemory; // Chalet
	VkDeviceMemory modelSceneryTextureMemory; // Scenery
	VkDeviceMemory checkedTextureMemory; // Chcked
	VkDeviceMemory frontSkyTextureMemory, backSkyTextureMemory, leftSkyTextureMemory, rightSkyTextureMemory, topSkyTextureMemory, bottomSkyTextureMemory; // Skybox
	// Image view which holds the texture image 
	// VkImageView which takes an image and is bound to a descriptor
	VkImageView textureImageView;
	VkImageView modelSceneryImageView;
	VkImageView modelChaletImageView;
	VkImageView checkedImageView;
	VkImageView skyboxImageView;
	// Texture sampler object that handles the texture sampler information - regards to how the image is presented - ie repeat or wrapped
	VkSampler textureSampler;
	// Depth image - like a colour attachment and defines the fepth of the images
	VkImage depthImage;
	// Depth image memory
	VkDeviceMemory depthImageMemory;
	// Depth image view - what part of the depth image we see
	VkImageView depthImageView;
};
