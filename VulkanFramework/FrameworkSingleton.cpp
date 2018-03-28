#include "FrameworkSingleton.h"

#include "include\STBIMAGE\stb_image.h"
#include "include\TinyOBJ\tiny_obj_loader.h"

FrameworkSingleton::FrameworkSingleton()
{
}

FrameworkSingleton::~FrameworkSingleton()
{
}

// Set instance to NULL initially, since it'll be created on demand in main
FrameworkSingleton* FrameworkSingleton::singletonInstance = NULL;

// Return windowMgr singleton instance
FrameworkSingleton* FrameworkSingleton::getInstance()
{
	// If it doesn't exist, create it
	if (singletonInstance == NULL)
	{
		singletonInstance = new FrameworkSingleton();
	}

	// Return created/existing instance
	return singletonInstance;
}

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
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Vec2
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	// Helper function used as part of the model loader to set the vertex information - Comment out when not using models
	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std //- Comment out when not using models
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

const std::vector<Vertex> cubeVertices1 =
{
	// Upper (original) square
	{ { -5.75f, 0.92f, 5.1f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.75f, 0.92f, 5.75f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.1f, 0.92f, 5.75f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -5.1f, 0.92f, 5.1f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

// Lower square 
{ { -5.75f, 0.12f, 5.1f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.75f, 0.12f, 5.75f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.1f, 0.12f, 5.75f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -5.1f, 0.12f, 5.1f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -5.1f, 0.92f, 5.75f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.75f, 0.92f, 5.75f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.75f, 0.12f, 5.75f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -5.1f, 0.12f, 5.75f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -5.1f, 0.12f, 5.1f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.75f, 0.12f, 5.1f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.75f, 0.92f, 5.1f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -5.1f, 0.92f, 5.1f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -5.75f, 0.12f, 5.1f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.75f, 0.12f, 5.75f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.75f, 0.92f, 5.75f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -5.75f, 0.92f, 5.1f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -5.1f, 0.12f, 5.75f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.1f, 0.12f, 5.1f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.1f, 0.92f, 5.1f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -5.1f, 0.92f, 5.75f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
};

const std::vector<Vertex> cubeVertices2 =
{
	// Upper (original) square
	{ { -5.0f, 0.62f, 4.5f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.0f, 0.62f, 5.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.5f, 0.62f, 5.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.5f, 0.62f, 4.5f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

// Lower square 
{ { -5.0f, 0.12f, 4.5f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.0f, 0.12f, 5.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.5f, 0.12f, 5.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.5f, 0.12f, 4.5f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -4.5f, 0.62f, 5.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.0f, 0.62f, 5.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.0f, 0.12f, 5.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.5f, 0.12f, 5.0f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -4.5f, 0.12f, 4.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.0f, 0.12f, 4.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.0f, 0.62f, 4.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -4.5f, 0.62f, 4.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -5.0f, 0.12f, 4.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.0f, 0.12f, 5.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.0f, 0.62f, 5.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -5.0f, 0.62f, 4.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -4.5f, 0.12f, 5.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.5f, 0.12f, 4.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -4.5f, 0.62f, 4.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -4.5f, 0.62f, 5.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
};

const std::vector<Vertex> cubeVertices3 =
{
	// Upper (original) square
	{ { -5.05f, 1.12f, 4.55f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.05f, 1.12f, 5.05f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.55f, 1.12f, 5.05f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.55f, 1.12f, 4.55f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

// Lower square 
{ { -5.05f, 0.62f, 4.55f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.05f, 0.62f, 5.05f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.55f, 0.62f, 5.05f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.55f, 0.62f, 4.55f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -4.55f, 1.12f, 5.05f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -5.05f, 1.12f, 5.05f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.05f, 0.62f, 5.05f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { -4.55f, 0.62f, 5.05f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

{ { -4.55f, 0.62f, 4.55f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.05f, 0.62f, 4.55f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.05f, 1.12f, 4.55f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -4.55f, 1.12f, 4.55f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -5.05f, 0.62f, 4.55f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -5.05f, 0.62f, 5.05f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -5.05f, 1.12f, 5.05f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -5.05f, 1.12f, 4.55f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

{ { -4.55f, 0.62f, 5.05f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -4.55f, 0.62f, 4.55f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { -4.55f, 1.12f, 4.55f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -4.55f, 1.12f, 5.05f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
};

// Vertivces for the Skybox
const std::vector<Vertex> skyboxVertices =
{
	// Upper (original) square
	{ { -250.0f, -250.0f, 250.0f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
{ { 250.0f, -250.0f, 250.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
{ { 250.0f, 250.0f, 250.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { -250.0f, 250.0f, 250.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },

// Lower square 
{ { -250.0f, -250.0f, -250.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
{ { 250.0f, -250.0f, -250.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
{ { 250.0f, 250.0f, -250.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
{ { -250.0f, 250.0f, -250.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
};

// Indices information which is used to draw a square 
const std::vector<uint32_t> planeIndices =
{
	3, 2, 1, 1, 0, 3
};

const std::vector<uint32_t> cubeIndices =
{
	0, 1, 2, 2, 3, 0, // Top
	4, 7, 6, 6, 5, 4, // Bottom
	8, 9, 10, 10, 11, 8, // Side 1
	12, 13, 14, 14, 15, 12, // Side 2 (opposite 1)
	16, 17, 18, 18, 19, 16,
	20, 21, 22, 22, 23, 20
};

// Indices for the Skybox
const std::vector<uint32_t> skyboxIndices =
{
	3, 2, 1, 1, 0, 3,
	7, 4, 5, 5, 6, 7,
	5, 4, 0, 0, 1, 5,
	6, 5, 1, 1, 2, 6,
	7, 6, 2, 2, 3, 7,
	4, 7, 3, 3, 0, 4
};

// Struct which stores details of swap chain support - query the physical device for some details 
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

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

// Struct which   Buffer Object 
struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

// VK_KHR_swapchain device extension which enables the swap chain via extension - similar to validation layers 
const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Enable a range of useful diagnostics layers instead of spefic ones 
const std::vector<const char*> validationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

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