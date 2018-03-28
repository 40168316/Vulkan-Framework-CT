#include "CameraManager.h"
#include "FrameworkSingleton.h" // Gives access to singleton and required libraries

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
}

// Method which initalises both free and target cameras
void CameraManager::initCameras()
{
	glfwSetInputMode(FrameworkSingleton::getInstance()->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Free camera for in game
	FrameworkSingleton::getInstance()->freeCam = new AllCamera::free_camera();
	FrameworkSingleton::getInstance()->freeCam->set_Posistion(glm::vec3(-15.0f, 5.0f, 0.0f));
	FrameworkSingleton::getInstance()->freeCam->rotate(0.0f, 0.0f);
	FrameworkSingleton::getInstance()->freeCam->set_Target(glm::vec3(0.0f, 0.0f, 0.0f));
	FrameworkSingleton::getInstance()->freeCam->set_projection(glm::quarter_pi<float>(), (float)FrameworkSingleton::getInstance()->WIDTH / (float)FrameworkSingleton::getInstance()->HEIGHT, 0.414f, 1000.0f);

	FrameworkSingleton::getInstance()->targetCamera = new AllCamera::target_camera();
	FrameworkSingleton::getInstance()->targetCamera->set_Posistion(glm::vec3(10.0f, 10.0f, 10.0f));
	FrameworkSingleton::getInstance()->targetCamera->set_Target(glm::vec3(0.0f, 0.0f, 0.0f));
	FrameworkSingleton::getInstance()->targetCamera->set_projection(glm::quarter_pi<float>(), (float)FrameworkSingleton::getInstance()->WIDTH / (float)FrameworkSingleton::getInstance()->HEIGHT, 0.414f, 40000.0f);

	FrameworkSingleton::getInstance()->cameraType = 0;
}