#include "CleanUpManager.h"
#include "FrameworkSingleton.h" // Gives access to singleton and required libraries

CleanUpManager::CleanUpManager()
{
}

CleanUpManager::~CleanUpManager()
{
}

// Once the window is closed, deallocation of resources occurs 
void CleanUpManager::cleanup()
{
	// Clean up and destroy the Swap Chain
	cleanupSwapChain();

	// Destory the image sampler
	vkDestroySampler(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->textureSampler, nullptr);
	// Destroy the texture image view
	vkDestroyImageView(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->textureImageView, nullptr);

	// Destory all VkImages
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->boxesTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->modelSceneryTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->modelChaletTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->checkedTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->frontSkyTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->backSkyTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->leftSkyTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->rightSkyTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->topSkyTexture, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->bottomSkyTexture, nullptr);

	// Destroy all VkImageMemory
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->boxesTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->modelSceneryTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->modelChaletTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->checkedTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->frontSkyTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->backSkyTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->leftSkyTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->rightSkyTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->topSkyTextureMemory, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->bottomSkyTextureMemory, nullptr);

	// Destroy the descriptor pool for the uniform buffers
	vkDestroyDescriptorPool(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->descriptorPool, nullptr);
	// Destroy the descriptor set layout used for the uniform buffers
	vkDestroyDescriptorSetLayout(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->descriptorSetLayout, nullptr);

	// Destroy and free the uniform buffer/memory
	vkDestroyBuffer(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->uniformBuffer, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->uniformBufferMemory, nullptr);
	vkDestroyBuffer(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->rotatingUniformBuffer, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->rotatingUniformBufferMemory, nullptr);

	// Destory the index buffer
	vkDestroyBuffer(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->indexPlane, nullptr);
	// Free the index buffer memory
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->indexPlaneMemory, nullptr);

	// Destroy the semaphore
	vkDestroySemaphore(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->imageAvailableSemaphore, nullptr);
	// Destroy the commandpool
	vkDestroyCommandPool(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->commandPool, nullptr);
	// Destory the framebuffer 
	for (auto framebuffer : FrameworkSingleton::getInstance()->swapChainFramebuffers)
	{
		vkDestroyFramebuffer(FrameworkSingleton::getInstance()->device, framebuffer, nullptr);
	}
	// Destroy the graphics pipeline 
	vkDestroyPipeline(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->graphicsPipeline, nullptr);
	vkDestroyPipeline(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->skyboxGraphicsPipeline, nullptr);
	// Destroy the pipeline
	vkDestroyPipelineLayout(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->pipelineLayout, nullptr);
	// Destory the render pass 
	vkDestroyRenderPass(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->renderPass, nullptr);
	// Destroy the imagesviews that are part of the swap chain 
	for (auto imageView : FrameworkSingleton::getInstance()->swapChainImageViews)
	{
		vkDestroyImageView(FrameworkSingleton::getInstance()->device, imageView, nullptr);
	}
	// Destory the swap chain 
	vkDestroySwapchainKHR(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->swapChain, nullptr);
	// Destroy the logical device 
	vkDestroyDevice(FrameworkSingleton::getInstance()->device, nullptr);
	// Destroy the debug report call back which relys any error messages through the use of validation layers 
	DestroyDebugReportCallbackEXT(FrameworkSingleton::getInstance()->instance, FrameworkSingleton::getInstance()->callback, nullptr);
	// Destroy the Vulkan surface object
	vkDestroySurfaceKHR(FrameworkSingleton::getInstance()->instance, FrameworkSingleton::getInstance()->surface, nullptr);
	// Destory the instance between the Vulkan Library and application just before the application closes
	vkDestroyInstance(FrameworkSingleton::getInstance()->instance, nullptr);
	// Destroy the window
	glfwDestroyWindow(FrameworkSingleton::getInstance()->window);
	// Terminate access to GLFW
	glfwTerminate();
}

// Clean Up old version of Swap Chain
void CleanUpManager::cleanupSwapChain()
{
	// Destroy and free up the image(view) and memory with regards to the depth buffer
	vkDestroyImageView(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->depthImageView, nullptr);
	vkDestroyImage(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->depthImage, nullptr);
	vkFreeMemory(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->depthImageMemory, nullptr);

	// Destroy all the framebuffers associated with the Swap Chain 
	for (size_t i = 0; i < FrameworkSingleton::getInstance()->swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->swapChainFramebuffers[i], nullptr);
	}

	// Free all the Command Buffers to the Command Pool associated with the Swap Chain
	vkFreeCommandBuffers(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->commandPool, static_cast<uint32_t>(FrameworkSingleton::getInstance()->commandBuffers.size()), FrameworkSingleton::getInstance()->commandBuffers.data());

	// Destroy the Graphics Pipeline and all information required for the pipeline - reverse order from how it was built
	vkDestroyPipeline(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->graphicsPipeline, nullptr);
	vkDestroyPipeline(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->skyboxGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->pipelineLayout, nullptr);
	vkDestroyRenderPass(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->renderPass, nullptr);

	// For all the Swap Cahin Image Views
	for (size_t i = 0; i < FrameworkSingleton::getInstance()->swapChainImageViews.size(); i++)
	{
		// Destroy
		vkDestroyImageView(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->swapChainImageViews[i], nullptr);
	}

	// Destroy the Swap Chain Object
	vkDestroySwapchainKHR(FrameworkSingleton::getInstance()->device, FrameworkSingleton::getInstance()->swapChain, nullptr);
}