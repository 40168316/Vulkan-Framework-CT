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
#include <stb_image.h>

// Include OBJ loader library
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
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


// Width and Heigh of the window - used throught the application 
const int WIDTH = 800;
const int HEIGHT = 600;

// Files used for the Models
const std::string MODEL_PATH = "models/sphere.obj"; // Set model
const std::string TEXTURE_PATH = "textures/texture.jpg"; // Set texture for model

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

// Struct called vertex which stores the information for the vertex shader
struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	// Tell Vulkan how to pass this data format to the vertex shader - bind the information
	static VkVertexInputBindingDescription getBindingDescription() 
	{
		// Struct which contains information regarding how the data is sumbitted to the GPU and vertex shader
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0; // Specifies the index of the binding in the array of bindings
		bindingDescription.stride = sizeof(Vertex); // Specifies the number of bytes from one entry to the next
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Specifies the move to the next data entry after each vertex

		return bindingDescription;
	}

	// Tell Vulkan the attribute description 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		// Two different attribute description, one for position
		attributeDescriptions[0].binding = 0; // Binding parameter specifies from which binding the per-vertex data comes
		attributeDescriptions[0].location = 0; // References the location directive of the input in the vertex shader.
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Describes the type of data for the attribute - vec3
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		// The other for colour 
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		// The other for the texture coordinates
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	//// Helper function used as part of the model loader to set the vertex information - Comment out when not using models
	//bool operator==(const Vertex& other) const 
	//{
	//	return pos == other.pos && color == other.color && texCoord == other.texCoord;
	//}
};

//namespace std - Comment out when not using models
//{
//	template<> struct hash<Vertex> 
//	{
//		size_t operator()(Vertex const& vertex) const 
//		{
//			return ((hash<glm::vec3>()(vertex.pos) ^
//				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//				(hash<glm::vec2>()(vertex.texCoord) << 1);
//		}
//	};
//}

// Position, Colour - data now as one part of vertices
const std::vector<Vertex> vertices =
{
	// Upper (original) square
	{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 1.0f, 0.0f }, {1.0f, 0.0f} },
	{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },

	// Lower square 
	{ { -0.5f, -0.5f, -1.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, -1.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, -1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

	//{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	//{ { 0.5f, -0.5f, -1.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	//{ { 0.5f, 0.5f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	//{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
};

// Indices information which is used to draw a square 
const std::vector<uint16_t> indices = 
{
	0, 1, 2, 2, 3, 0, // Top
	4, 5, 6, 6, 7, 4, // Bottom
	4, 5, 1, 1, 0, 4, // Sides
	5, 6, 2, 2, 1, 5, // Sides
	6, 7, 3, 3, 2, 6, // Sides
	7, 4, 0, 0, 3, 7 // Sides
};

// Struct which Uniform Buffer Object 
struct UniformBufferObject 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

// Class which contains the Vulkan Objects
class VulkanObjects 
{
	// Public modifier - can be seen by all
public:
	AllCamera::free_camera* freeCam;
	// Create vector to apply to current cam pos
	glm::vec3 freeCamPos = glm::vec3(0, 0, 0);
	double cursor_x, cursor_y = 0.0;
	double dt;
	
	// Run method which contains all the private class members 
	void run() 
	{
		initWindow();
		initVulkan();
		initAdditionalFeatures();
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
	// Vectors which contain the vertices and indices for the model
	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;
	// Vertex Buffer object 
	VkBuffer vertexBuffer;
	// Vertex Buffer memory object which holds the memory regarding the vertex buffer 
	VkDeviceMemory vertexBufferMemory;
	// Index buffer object
	VkBuffer indexBuffer;
	// Vertex Buffer memory object which holds the memory regarding the vertex buffer 
	VkDeviceMemory indexBufferMemory;
	// Descriptor layout used for specifying the layout for the uniform buffers
	VkDescriptorSetLayout descriptorSetLayout;
	// Uniform buffer object which is used to store the uniform buffer
	VkBuffer uniformBuffer;
	// Uniform buffer object memory 
	VkDeviceMemory uniformBufferMemory;
	// Descriptor pool object which is used to get descriptor sets
	VkDescriptorPool descriptorPool;
	// Descriptor set which is gets sets from the pool
	VkDescriptorSet descriptorSet;
	// Object which is used to store texture images 
	VkImage textureImage;
	// Texture image memory 
	VkDeviceMemory textureImageMemory;
	// Image view which holds the texture image 
	VkImageView textureImageView;
	// Texture sampler object that handles the texture sampler information - regards to how the image is presented - ie repeat or wrapped
	VkSampler textureSampler;
	// Depth image - like a colour attachment and defines the fepth of the images
	VkImage depthImage;
	// Depth image memory
	VkDeviceMemory depthImageMemory;
	// Depth image view - what part of the depth image we see
	VkImageView depthImageView;

	void initAdditionalFeatures()
	{
		// Free camera for in game
		freeCam = new AllCamera::free_camera();
		freeCam->set_Posistion(glm::vec3(4.0f, 4.0f, 4.0f));
		freeCam->rotate(-10.0, 0.0);
		freeCam->set_Target(glm::vec3(0, 0, 0));
		freeCam->set_projection(glm::quarter_pi<float>(), (float)WIDTH/(float)HEIGHT, 0.414f, 40000.0f);
	}

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
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createFramebuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		//loadModel();			// Method used to load model
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffer();
		createDescriptorPool();
		createDescriptorSet();
		createCommandBuffers();
		createSemaphores();
	}

	// Function which loads a model
	//void loadModel() 
	//{
	//	// Attribute container which holds all of the position, texture coordinates and faces
	//	tinyobj::attrib_t attrib;
	//	std::vector<tinyobj::shape_t> shapes; // Shapes
	//	std::vector<tinyobj::material_t> materials; // The materials
	//	std::string err; // Any errors or warnings that can occur while the model is in transit - loading...

	//	// If the model cannot be loaded then throw an error
	//	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str())) 
	//	{
	//		throw std::runtime_error(err);
	//	}

	//	// Unordered map which stores all of the unique vertices 
	//	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	//	// Iterate over all the shapes which make up the model
	//	for (const auto& shape : shapes) {
	//		// For all the incides in the model
	//		for (const auto& index : shape.mesh.indices) 
	//		{
	//			// Find the vertex positions
	//			Vertex vertex = {};
	//			vertex.pos = {
	//				attrib.vertices[3 * index.vertex_index + 0],
	//				attrib.vertices[3 * index.vertex_index + 1],
	//				attrib.vertices[3 * index.vertex_index + 2]
	//			};

	//			// Find the vertex texture coordinates
	//			vertex.texCoord = {
	//				attrib.texcoords[2 * index.texcoord_index + 0],
	//				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	//			};

	//			// Set the vertex colour
	//			vertex.color = { 1.0f, 1.0f, 1.0f };

	//			//
	//			if (uniqueVertices.count(vertex) == 0) 
	//			{
	//				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
	//				vertices.push_back(vertex);
	//			}

	//			indices.push_back(uniqueVertices[vertex]);
	//		}
	//	}
	//}

	void createDepthResources() 
	{
		// Find the depth format that will be used
		VkFormat depthFormat = findDepthFormat();

		// Call the create image and depth image view functions now that we know what formats of depth buffer are supported 
		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		// Transition to the image layout passing the depth image and format information to produce the depth buffering effect
		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	// Function which finds the supported format based on the tiling mode and usuage - physical device is checked for support
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
	{
		// For all candiates
		for (VkFormat format : candidates) 
		{
			// Get the properties required to be compared against the physical device - checks for linear and optimal tiling features along with buffer features
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			// If tiling type linear is supported then return format
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			}
			// Else if tiling type optimal is supported then return format
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
				return format;
			}
		}
		// Else throw an error
		throw std::runtime_error("failed to find supported format!");
	}

	// Helper function which is used to find the depth and format for the depth buffer
	VkFormat findDepthFormat() 
	{
		// Return the supported format from the find supported format function with all its data 
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// Function which checks if the format has a stencil component
	bool hasStencilComponent(VkFormat format) {

		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	// Function which specifies the type of texture sampler that will be used 
	void createTextureSampler() 
	{
		// Struct which specifies the sampler information
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; // Specify the type of struct 
		samplerInfo.magFilter = VK_FILTER_LINEAR; // specify how to interpolate texels that are magnified - set to linear
		samplerInfo.minFilter = VK_FILTER_LINEAR; // specify how to interpolate texels that are minified - set to linear 
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // X - set the axis to repeat - means to stretching or wrapping the texture
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // Y - set the axis to repeat
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // Z - set the axis to repeat
		samplerInfo.anisotropyEnable = VK_TRUE; // Set anisotropy filtering to true
		samplerInfo.maxAnisotropy = 16; // Set the range to 16
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Set the color which is returned when sampling beyond the image
		samplerInfo.unnormalizedCoordinates = VK_FALSE; // Specifies which coordinate system you want to use to address texels in an image - false means range of 0 to 1
		samplerInfo.compareEnable = VK_FALSE; // Set compare enable to false
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; 
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Set mip map mode to linear 
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		// Initiate the texture sampler - if not successful throw an error 
		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	// Function which is used to create a texture view for an image - used as part of the graphics pipeline and in the swap chain process 
	void createTextureImageView() 
	{
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	// Function which creates and returns an image view
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		// Struct which contains information regarding the creation of the image view 
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // Set struct type
		viewInfo.image = image; // Image to image
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Image dimensions to one
		viewInfo.format = format; // Format to format 
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		// Image view object
		VkImageView imageView;
		// Initiate the image view - if not successful throw an error
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		// Return the image view 
		return imageView;
	}

	// Function which will load an image and upload it into a Vulkan image object
	void createTextureImage() 
	{
		// Use the STBI image loader to load the image and 
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		// If pixels do not exist then throw error 
		if (!pixels) 
		{
			throw std::runtime_error("failed to load texture image!");
		}

		// Staging buffer used for copying the pixels from an image to the buffer
		VkBuffer stagingBuffer;
		// Staging buffer memory 
		VkDeviceMemory stagingBufferMemory;
		// Create the buffer based on the image size and the staging buffer
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy the pixel values directly that were obtained from the image loading library to the buffer
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		// Clean up the original pixel array 
		stbi_image_free(pixels);

		// Create the image by inputing the image and getting all the pixel information
		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		// Transition the image to the texture
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		// Copy the buffer
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// Transition the image to the texture, however, this time with shader access
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Destroy and free the buffer/memory  
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Function which copies the buffer to the image
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
	{
		// Start recording the command buffer
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Struct which specifies the region of the image 
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		// Copy the buffer itself 
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// End the reocrding of the command buffer 
		endSingleTimeCommands(commandBuffer);
	}

	// Function which beings the recording of the command buffer
	VkCommandBuffer beginSingleTimeCommands() 
	{
		// Struct which temporarily allocates a command buffer to allow for memory transfer operations
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // Set type to command pool
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Set to primary to only allow memory transfer
		allocInfo.commandPool = commandPool; // Create a command pool to generate command buffers
		allocInfo.commandBufferCount = 1; // Set the count to one as only one is required

		// Allocate the command buffer from the command pool in the struct 
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		// Start recording the command buffer to record the memory transfer
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // Set command buffer to begin
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Set to one at a time as there is only one command buffer

		// Begin recording the Command Buffer
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		// Return the Command Buffer
		return commandBuffer;
	}

	// Function which ends the recording of the command buffer
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) 
	{
		// End the recording of the command bufer
		vkEndCommandBuffer(commandBuffer);

		// Execute the command buffer to complete the transfer of memory 
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Sumbit the information to the queue and wait until finished
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		// Free the command buffer now transfer is complete 
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	// Function which is used to create image based on the contents inside the vulkan image object 
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) 
	{
		// Struct which specifies image information such as 
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // Set the type of struct
		imageInfo.imageType = VK_IMAGE_TYPE_2D; // Set the image type to 2D
		imageInfo.extent.width = width; // Set the width tp the width of the window
		imageInfo.extent.height = height; // Set the height tp the width of the window
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format; // Set format to the value passed in
		imageInfo.tiling = tiling; // Set tiling to the value passed in 
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Set the intial layout to not usable by the GPU and the very first transition will discard the texels
		imageInfo.usage = usage; // Set usage to the value passed in 
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Iniate the create image - if unsuccessful throw error
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image!");
		}

		// Specify the memroy requirements required for the image
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		// Struct which is used to store information regarding the allocation of memory 
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		// Allocate the image memory - if not successful throw an error 
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		// Bind the image and the image memory 
		vkBindImageMemory(device, image, imageMemory, 0);
	}

	// Function which is used to create the descriptor sets from the descriptor pool 
	void createDescriptorSet() 
	{
		VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
		// Struct which contains information regarding the sets
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool; // Specfify the pool from which the descriptor sets are assigned from
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		// Initiate the descriptor sets - if not successful throw error 
		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		// Struct specifies the buffer and the region within it that contains the data for the descriptor
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Struct which contains infomration with regards to the image - bding the imag and rampler using the descriptor 
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		// Create an array of descriptors 
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // Set type to descriptor write 
		descriptorWrites[0].dstSet = descriptorSet; // Assign the created descriptor set
		descriptorWrites[0].dstBinding = 0; // Binding index starts at the first element - 0
		descriptorWrites[0].dstArrayElement = 0; // Binding index starts at the first element - 0
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //Define the descriptor as uniform buffer 
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo; // Set the buffer info

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // Set type to descriptor write 
		descriptorWrites[1].dstSet = descriptorSet; // Assign the created descriptor set
		descriptorWrites[1].dstBinding = 1; // Binding index starts at the second element - 1
		descriptorWrites[1].dstArrayElement = 0; // Binding index starts at the first element - 0
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //Define the descriptor type as combined image sampler 
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo; // Set the image info

		// Update the descriptor sets based on the data provided in the structs above 
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	// Function which contains the descriptor pools which is used to allocate a descriptor set - like command buffers
	void createDescriptorPool()
	{
		// Array of descriptor pools 
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Pool 0 to uniform buffers
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Pool 1 to image sampler
		poolSizes[1].descriptorCount = 1;

		// Struct which contains information regarding the sets in the pool
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		// Initiate descriptor pool - if fail throw error
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// Function which updates the uniform buffer with a new transformation every frame 
	void createUniformBuffer() 
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
	}


	// Function which provides details about every descriptor binding used in the shaders for pipeline creation - MVP
	void createDescriptorSetLayout() 
	{
		// Define the biding of the uniform buffer object 
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		// Set the shader stage for the descriptor to vertex 
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// Struct which contains information about the layout binding 
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Array which binds the uniform buffer objects and the sampler layouts
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		// Struct which contains information regarding the binding of the descriptors
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // Type
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // Bind count
		layoutInfo.pBindings = bindings.data(); // Bind data 

		// Bind the descriptor - if not sucessful throw an error
		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	// Function which handles in index buffer - using the vertex data and various buffers to change a triangle to a square
	void createIndexBuffer() 
	{
		// Culculate the buffer size based on the number of incidies 
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		// Create a staging buffer which will stage the data 
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copying the vertex data to the buffer - done by mapping the buffer memory into the CPU 
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // Map the data to the memory (logical device, staging buffer memory memory, offset, size, specify flags, data)
		memcpy(data, indices.data(), (size_t)bufferSize); // Memory copy the indicy data to the mapped memory then unmap the memory
		vkUnmapMemory(device, stagingBufferMemory);

		// Create a buffer using the index information
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		// Copy botht the staging and index buffer 
		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		// Destroy and free the staging buffers 
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card - in this case, storing vertex data
	void createVertexBuffer() 
	{
		// Calculate the buffer size based on the number of vertices 
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		// Create a staging buffer which is used for copying the vertex data. 
		VkBuffer stagingBuffer;
		// Staging buffer memory which handles the variable memory size of the staging buffer 
		VkDeviceMemory stagingBufferMemory;
		// Call the create buffer function pass the required staging information required
		createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copying the vertex data to the buffer - done by mapping the buffer memory into the CPU 
		void* data; 
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // Map the data to the memory (logical device, VB memory, offset, size, specify flags, data)
		memcpy(data, vertices.data(), (size_t) bufferSize); // Memory copy the vertex data to the mapped memory then unmap the memory
		vkUnmapMemory(device, stagingBufferMemory);

		// Call the create buffer function pass the required vertex information required
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		// Copy both buffers to the Device Logical buffer
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// Destory and then free the staging buffer
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// Function which is called apon to create buffers with data passed in such as vertex or fragment
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		// Struct which contains information about the Vertex Buffer
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size; // Specifies the size of the buffer in bytes using the parameter sizeof
		bufferInfo.usage = usage; // Indicates for which purposes the data in the buffer is going to be used - vertex buffer usage in this case
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Specify the buffer can only be used by the graphics queue - making in exclusive 

		// Initiate the Vertex Buffer object using the logical device, vertex buffer - if unsuccessful throw an error
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		// Create an object which defines the memory requirements for the vertex buffer 
		VkMemoryRequirements memRequirements;
		// Object which gets buffer memory requirements taking in logical device, vertex buffer object and memReuirements object 
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		// With the correct memory determined using findMemoryType function, the actual memory allocation can occur 
		// Struct which allocates the memory for the vertex buffer
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size; // Define the size which was derived from the memory requirements
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // Set the type

		// Initialise the memory for the vertex buffer - if unsuccessful throw an error 
		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		// Bind the vertex buffer and the memory 
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	// Function which copies the contents from one buffer to another 
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
	{
		// Set the command buffer to start recording - contained in another function 
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Copy the buffer - use a struct to calculate the size
		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		// Actually copy the buffer
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// End the recording of the command buffer 
		endSingleTimeCommands(commandBuffer);
	}

	// Function which deals with Layout Transitions 
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
	{
		// Begin the recording of the command buffer
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Struct which holds the information about an image memory barrier. 
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // Set type of struct to image memory barrier
		barrier.oldLayout = oldLayout; // Set the oldlayout to the layout passed in
		barrier.newLayout = newLayout; // Set the new layout to the layout passed in - part of the layout transition
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Set queue family indexes to ignore
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Set queue family indexes to ignore
		barrier.image = image; // Set the barrier image to the image

		// If new layout is equal to the stencil attachment then
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			// Change the aspect mask to aspect depth bit
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			// If the format contains the stencil component
			if (hasStencilComponent(format)) 
			{
				// Update the mask once more to stencil 
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else 
		{
			// Else set the aspect mask to aspect color bit
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1; 

		// Pipeline flags which store information regarding what stage the pipeline is at 
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// Transition Layout possibilities
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		// Else then not a valid transition type so throw error 
		else 
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		// Create the pipeline barrier which is used to synchronize access to resources with regards to the image 
		vkCmdPipelineBarrier
		(
			commandBuffer, // Command Buffer
			sourceStage, destinationStage, // 
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// End the recording of the command buffer
		endSingleTimeCommands(commandBuffer);
	}

	// Function which defines the memory type as graphics cards can vary on different types of memories they offer 
	// This is done by combing the requirements of the buffer and our our own application requirements to find the right type of memory 
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
		// Query for the correct types of memory which can be found on the selected physical device 
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		// For the memory type count found
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			// If typefilter - which specifies the bit field of memory types that are suitable = true - and if the property flags and properties of the physical device = desired properties 
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}
		// Throw an error if no suitable types of memory were found 
		throw std::runtime_error("failed to find suitable memory type!");
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

		// Struct which specifies the depth buffering information as an attachment 
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat(); // Get the format of the depth buffer
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Struct colorAttachmentRef which details the colour attachment type 
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Gives the best performance 

		// Struct which details the depth buffering attachment 
		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// A single render pass can consist of multiple subpasses. 
		// Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes. 
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		// Include the colour attachment struct above 
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

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

		// Array of the colour and dpeth attachments required for the render pass 
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		// Render pass struct which details the render pass information and includes other structs 
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		// Include the color attachment struct 
		renderPassInfo.pAttachments = attachments.data();
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
		for (uint32_t i = 0; i < swapChainImages.size(); i++) 
		{
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	// Method which recreates the swap chain - deals with changes to the window such as window resizing
	void recreateSwapChain() 
	{
		// Call device wait idle to make sure we dont access the logical device until it has stopped doing something - if this check is in place the Swap Chain can be corrupted. 
		vkDeviceWaitIdle(device);

		// Before Swap Chain can be recreate old version has to be cleaned up
		cleanupSwapChain();

		// Recreate the Swap Chain itself 
		createSwapChain();
		// Recreate the image view because they are based difrectly on the swap chain images
		createImageViews();
		// The render pass is recreated because it depends on the format of the swap chain images - rare but check just incase
		createRenderPass();
		// Recreate the graphics pipeline due to the fact the viewport and scissor size may have been changed hence rebuilidng is required 
		createGraphicsPipeline();
		// Recreate the depth buffers
		createDepthResources();
		// Recreate all buffers as they are based on the swap chain images 
		createFramebuffers();
		createCommandBuffers();
	}

	// Clean Up old version of Swap Chain
	void cleanupSwapChain() 
	{
		// Destroy and free up the image(view) and memory with regards to the depth buffer
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		// Destroy all the framebuffers associated with the Swap Chain 
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) 
		{
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}

		// Free all the Command Buffers to the Command Pool associated with the Swap Chain
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		
		// Destroy the Graphics Pipeline and all information required for the pipeline - reverse order from how it was built
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		// For all the Swap Cahin Image Views
		for (size_t i = 0; i < swapChainImageViews.size(); i++) 
		{
			// Destroy
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		// Destroy the Swap Chain Object
		vkDestroySwapchainKHR(device, swapChain, nullptr);
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;

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

		// Check the supported features for the physical device 
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
			

			// Start the time in seconds as the rendering has started with floating point accuracy - required for movement - like delta time
			static auto startTime = std::chrono::high_resolution_clock::now();
			// Get the current time
			auto currentTime = std::chrono::high_resolution_clock::now();
			// Calculate the current time by taking the start time away from the current time
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			if (glfwGetKey(window, GLFW_KEY_W))
			{
				std::cout << "W" << std::endl;
				freeCamPos = (glm::vec3(0, 0, 5.0f * time));
			}
			if (glfwGetKey(window, GLFW_KEY_A))
			{
				std::cout << "A" << std::endl;
				freeCamPos = (glm::vec3(-5.0f, 0, 0));
			}
			if (glfwGetKey(window, GLFW_KEY_S))
			{
				std::cout << "S" << std::endl;
				freeCamPos = (glm::vec3(0, 0, -5.0f));
			}
			if (glfwGetKey(window, GLFW_KEY_D))
			{
				std::cout << "D" << std::endl;
				freeCamPos = (glm::vec3(5.0f, 0, 0));
			}
			freeCam->move(freeCamPos);

			// Free cam stuff
			static double ratio_width = glm::quarter_pi<float>() / WIDTH;
			static double ratio_height = (glm::quarter_pi<float>() * (WIDTH / HEIGHT)) / static_cast<float>(WIDTH);
			double current_x, current_y;
			glfwGetCursorPos(window, &current_x, &current_y);
			// Calc delta
			double delta_x = current_x - cursor_x;
			double delta_y = current_y - cursor_y;
			// Multiply deltas by ratios
			delta_x *= ratio_width;
			delta_y *= ratio_height * -1; // -1 to invert on y axis
										  // Rotate camera by delta
			freeCam->rotate(delta_x, delta_y);
			freeCam->update(0.001);
			// Update cursor pos
			cursor_x = current_x;
			cursor_y = current_y;

			
			glfwPollEvents();

			// Update the uniform buffer to allow for transforms to take place 
			updateUniformBuffer();
			drawFrame();
		}

		// Wait for logical device to finish before destorying 
		vkDeviceWaitIdle(device);
	}

	// Function which is called as part of the main loop which updates geometry
	void updateUniformBuffer() 
	{
		// Start the time in seconds as the rendering has started with floating point accuracy - required for movement - like delta time
		static auto startTime = std::chrono::high_resolution_clock::now();
		// Get the current time
		auto currentTime = std::chrono::high_resolution_clock::now();
		// Calculate the current time by taking the start time away from the current time
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// Struct which contains the Model view projection matrix information stored in the uniform buffer object
		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Multiple radian * time part by 0.01f to go really really slow 
		ubo.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Camera distance, focus point, up axis
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f); // 45 degree field of view, aspect ratio, near and far view planes
		ubo.proj[1][1] *= -1;

		//ubo.view = freeCam->get_View();
		//ubo.proj = freeCam->get_Projection();

		// Once the MVP is set, copy the uniform data over
		void* data;
		vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBufferMemory);
	}

	// Method which deals with acquiring an image from the swap chain, execute the command buffer and returns the image to the swap chain for presentation
	void drawFrame() 
	{
		uint32_t imageIndex;
		// Acquire the next image from the swap chain using the logical device, swaphcain, timeout in nanoseconds, the semaphore, handle and reference to image index
		VkResult result =  vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		// Check if the Swap Chain is no longer compatibile - out of date 
		if (result == VK_ERROR_OUT_OF_DATE_KHR) 
		{
			// Create and update
			recreateSwapChain();
			return;
		}
		// Else of the The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched exactly - cause by resizing the image sometimes
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

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
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// If the result of the request made above is invalid or no longer compatible - out of date - then recreate the swap chain 
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		{
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

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

	// Function which deals with the window resizing 
	static void onWindowResized(GLFWwindow* window, int width, int height) 
	{
		// If width or hieght equal 0 then return false
		if (width == 0 || height == 0) return;

		// If the window size is not zero then resize the window and recreate the swap chain 
		VulkanObjects* app = reinterpret_cast<VulkanObjects*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
	}

	// Once the window is closed, deallocation of resources occurs 
	void cleanup() 
	{
		// Clean up and destroy the Swap Chain
		cleanupSwapChain();

		// Destory the image sampler
		vkDestroySampler(device, textureSampler, nullptr);
		// Destroy the texture image view
		vkDestroyImageView(device, textureImageView, nullptr);
		// Destory and free texture image object/memory 
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		// Destroy the descriptor pool for the uniform buffers
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		// Destroy the descriptor set layout used for the uniform buffers
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Destroy and free the uniform buffer/memory
		vkDestroyBuffer(device, uniformBuffer, nullptr);
		vkFreeMemory(device, uniformBufferMemory, nullptr);

		// Destory the index buffer
		vkDestroyBuffer(device, indexBuffer, nullptr);
		// Free the index buffer memory
		vkFreeMemory(device, indexBufferMemory, nullptr);

		// Destroy the vertex buffer
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		// Free the vertex buffer memory
		vkFreeMemory(device, vertexBufferMemory, nullptr);

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
		// If current width is not equal to the maximum size then return current extent 
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		// Else get width and height of window and set
		else 
		{
			int width, height;
			glfwGetWindowSize(window, &width, &height);

			VkExtent2D actualExtent = 
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

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
		// Get the binding and attribute information setup in the vertex struct
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		// Depth values is not used so set to false 
		rasterizer.depthBiasEnable = VK_FALSE;

		// Multisampling which is one of the ways to perform anti-aliasing
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE; // Set to false as multisampling is not used 
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Depth Stencil which is used as part of the depth buffer 
		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; // Define struct type
		depthStencil.depthTestEnable = VK_TRUE; // Set the depth of new fragments should be compared to the depth buffer to see if they should be discarded
		depthStencil.depthWriteEnable = VK_TRUE; // Set the new depth of fragments that pass the depth test should actually be written to the depth buffer
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Specifies the comparison that is performed to keep or discard fragments
		depthStencil.depthBoundsTestEnable = VK_FALSE; // Set additional optional bound test to false
		depthStencil.stencilTestEnable = VK_FALSE; // Set the additional optional stencil test to false also

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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;


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
		pipelineInfo.pDepthStencilState = &depthStencil;
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

		// Iterate through the image views and create framebuffers from them - both the swap chain and depth buffers 
		for (size_t i = 0; i < swapChainImageViews.size(); i++) 
		{
			// An array of the attachments 
			std::array<VkImageView, 2> attachments =
			{
				swapChainImageViews[i],
				depthImageView
			};

			// Create a struct which stores the info of the framebuffer
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// Specify the render pass the framebuffer requires to be compatible 
			framebufferInfo.renderPass = renderPass;
			// Set attachment count as one as there is only the coulour attachment
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			// Specify the attachments the framebuffer requires to be compatible
			framebufferInfo.pAttachments = attachments.data();
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

			// Array which clears the colours and also allows for the background colour to be set and the depth of the depth buffer 
			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			// Begin the render pass using the struct created above 
			// Command buffer to record the command to, render pass struct, controls how the drawing commands within the render pass will be provided - INLINE
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind the graphics pipeline 
			// Command buffer to record the command to, pipeline object is a graphics pipeline, 
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			// Get the vertex buffer information
			VkBuffer vertexBuffers[] = { vertexBuffer };
			// Specify the offset - not existing in this case
			VkDeviceSize offsets[] = { 0 };
			// Bind the vertex buffers - commandbuffers, offset, number of bindings, vertexbuffers themselves and offests of the vertex data
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			// Bind the index buffers
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16); // VK_INDEX_TYPE_UINT32 - needs to be 16 when using models
			// Bind the descriptor sets 
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

			// Draw the command buffers (vertex count, instanceCount, firstVertex, firstInstance)
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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