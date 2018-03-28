#pragma once

#include "CleanUpManager.h"

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
#include <thread>
#include <omp.h>

struct Vertex;
struct SwapChainSupportDetails;
struct QueueFamilyIndices;
struct UniformBufferObject;

class VulkanManager
{
public:
	VulkanManager();
	~VulkanManager();

	CleanUpManager cleanUpManager;

	void initVulkan();
	void loadModel(std::string modelPath, std::vector<Vertex> &modelVertices, std::vector<uint32_t> &modelIndices);
	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	void createTextureSampler();
	void createTextureImageView(VkImage texture, VkImageView &textureImView, VkImageViewType &imageType);
	void createCubeTextureImageView(VkImage texture1, VkImage texture2, VkImage texture3, VkImage texture4, VkImage texture5, VkImage texture6, VkImageView &textureImView, VkImageViewType &imageType);
	VkImageView createCubeImageView(VkImage image1, VkImage image2, VkImage image3, VkImage image4, VkImage image5, VkImage image6, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType &imageType);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType &imageType);
	void createTextureImage(std::string textureName, VkImage &textureIm, VkDeviceMemory &textureImMemory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void createDescriptorSet(VkDescriptorSet &desSet, VkImageView textureImView, VkBuffer uniformBuff);
	void createDescriptorPool();
	void createUniformBuffer(VkBuffer &uniformBuff, VkDeviceMemory &uniformBuffMemory);
	void createDescriptorSetLayout();
	void createIndexBuffer(std::vector<uint32_t> shape, VkBuffer &shapeIndexBuffer, VkDeviceMemory &shapeIndexBufferMemory);
	void createVertexBuffer(std::vector<Vertex> vertexInformation, VkBuffer &shapeVertexBuffer, VkDeviceMemory &shapeVertexBufferMemory);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createSemaphores();
	void createRenderPass();
	void createImageViews();
	void recreateSwapChain();
	void createSwapChain();
	void createSurface();
	void createLogicalDevice();
	void pickPhysicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void setupDebugCallback();
	void mainLoop();
	void updateUniformBuffer(VkDeviceMemory uniformBuffMemory);
	void drawFrame();
	static void onWindowResized(GLFWwindow* window, int width, int height);
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	static std::vector<char> readFile(const std::string& filename);
	void createSkyboxGraphicsPipeline(std::string vertPath, std::string fragPath);
	void createGraphicsPipeline(std::string vertPath, std::string fragPath);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFramebuffers();
	void createCommandBuffers();
	void createCommandPool();

	//void modelLoad(std::vector<tinyobj::shape_t> shapes, tinyobj::attrib_t attrib, std::unordered_map<Vertex, uint32_t> uniqueVertices, std::vector<Vertex> &modelVertices, std::vector<uint32_t> &modelIndices);
};