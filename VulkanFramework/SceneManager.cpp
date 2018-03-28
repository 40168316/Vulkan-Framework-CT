#include "SceneManager.h"
#include "FrameworkSingleton.h" // Gives access to singleton and required libraries

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::input()
{

	// Only if camera type is free camera, allow it to move and update
	if (FrameworkSingleton::getInstance()->cameraType == 0)
	{
		// Move the free camera position vector
		// W moves the camera forward
		if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_W))
		{
			FrameworkSingleton::getInstance()->freeCamPos = (glm::vec3(0.0f, 0.0f, 5.0f)*FrameworkSingleton::getInstance()->cameraSpeed);
		}
		// A moves the camera left
		if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_A))
		{
			FrameworkSingleton::getInstance()->freeCamPos = (glm::vec3(-5.0f, 0.0f, 0.0f)*FrameworkSingleton::getInstance()->cameraSpeed);
		}
		// S moves the camera backwards
		if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_S))
		{
			FrameworkSingleton::getInstance()->freeCamPos = (glm::vec3(0.0f, 0.0f, -5.0f)*FrameworkSingleton::getInstance()->cameraSpeed);
		}
		// D moves the camera right
		if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_D))
		{
			FrameworkSingleton::getInstance()->freeCamPos = (glm::vec3(5.0f, 0.0f, 0.0f)*FrameworkSingleton::getInstance()->cameraSpeed);
		}
		// Update the free camera by the free camera position
		FrameworkSingleton::getInstance()->freeCam->move(FrameworkSingleton::getInstance()->freeCamPos);
	}

	// Change between target camera and free camera
	// If f is pressed then change to free camera
	if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_F))
	{
		FrameworkSingleton::getInstance()->cameraType = 0;
	}

	// If 1 is pressed then change to target camera and set new position
	if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_1))
	{
		FrameworkSingleton::getInstance()->cameraType = 1;
		FrameworkSingleton::getInstance()->targetCamera->set_Posistion(glm::vec3(10.0f, 10.0f, 10.0f));
	}
	// If 2 is pressed then change to target camera and set new position
	if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_2))
	{
		FrameworkSingleton::getInstance()->cameraType = 1;
		FrameworkSingleton::getInstance()->targetCamera->set_Posistion(glm::vec3(-10.0f, 10.0f, 10.0f));
	}
	// If 3 is pressed then change to target camera and set new position
	if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_3))
	{
		FrameworkSingleton::getInstance()->cameraType = 1;
		FrameworkSingleton::getInstance()->targetCamera->set_Posistion(glm::vec3(-10.0f, 10.0f, -10.0f));
	}
	// If 4 is pressed then change to target camera and set new position
	if (glfwGetKey(FrameworkSingleton::getInstance()->window, GLFW_KEY_4))
	{
		FrameworkSingleton::getInstance()->cameraType = 1;
		FrameworkSingleton::getInstance()->targetCamera->set_Posistion(glm::vec3(10.0f, 10.0f, -10.0f));
	}
}



void SceneManager::update() 
{
	// Reset the free camera position every frame
	FrameworkSingleton::getInstance()->freeCamPos = glm::vec3(0.0f, 0.0f, 0.0f);

	// CALCULATE FPS
	// Number of frames/samples used to calculate the fps
	static const int numSamples = 10;
	// An array of floats storing 10 frametimes
	static float frameTimes[numSamples];
	// CurrentFrame - set to 0 and increment frame by frame
	static int currentFrame = 0;
	// Get the prevticks(time) by getting the glfw time
	static float prevTicks = glfwGetTime();
	// Variable called current ticks, set to nothing which will be assigned prevticks at a later stage
	float currentTicks;
	// Get the current ticks(time) by using glfw time
	currentTicks = glfwGetTime();

	// Set frame time to currentTicks - previousTicks as this is the definition of a frame
	frameTime = currentTicks - prevTicks;
	// Wrap the array, so once the last index has been filled, refill from the first
	frameTimes[currentFrame % numSamples] = frameTime;
	// Make previousTick equal to current ticks and the last frame time has been added to the array
	prevTicks = currentTicks;
	
	// Count which will be used to track if the array has been initial filled or not
	int count;
	// If the currentframe count value is lower than the numsamples (ie the array has empty values)
	if (currentFrame < numSamples)
	{
		// Set count to the currentframe 
		count = currentFrame;
	}
	else 
	{
		// Else set it to the number of sames
		count = numSamples;
	}

	// Get the average frametime from the 10 frame values in the array
	float frameTimeAverage = 0;
	// For loop to loop through all the currentframes or num samples
	for (int i = 0; i < count; i++)
	{
		// Add up all the values in the array
		frameTimeAverage += frameTimes[i];
	}
	// Get the average of the added up values
	frameTimeAverage /= count;

	// If the frame time average is greater than zero then
	if (frameTimeAverage > 0)
	{
		// Make FPS equal to 1.0f (second - note glfwgettime is return in seconds not milliseconds so a frame value in the 1000s is to be expected) divided by the frameTimeAverage
		FPS = 1.0f / frameTimeAverage;
	}

	// Increment the current frame
	currentFrame++;
	// Output the FPS value
	std::cout << FPS << " FPS" << std::endl;

	// Free cam stuff
	static double ratio_width = glm::quarter_pi<float>() / FrameworkSingleton::getInstance()->WIDTH;
	static double ratio_height = (glm::quarter_pi<float>() * (FrameworkSingleton::getInstance()->WIDTH / FrameworkSingleton::getInstance()->HEIGHT)) / static_cast<float>(FrameworkSingleton::getInstance()->WIDTH);
	double current_x, current_y;
	glfwGetCursorPos(FrameworkSingleton::getInstance()->window, &current_x, &current_y);
	// Calc delta
	double delta_x = current_x - FrameworkSingleton::getInstance()->cursor_x; //op
	double delta_y = current_y - FrameworkSingleton::getInstance()->cursor_y;
	// Multiply deltas by ratios
	delta_x *= ratio_width;
	delta_y *= ratio_height * -1; // -1 to invert on y axis
	// Rotate camera by delta 
	FrameworkSingleton::getInstance()->freeCam->rotate(delta_x, delta_y);
	FrameworkSingleton::getInstance()->freeCam->update(0.001);
	FrameworkSingleton::getInstance()->targetCamera->update(0.00001);
	// Update cursor pos
	FrameworkSingleton::getInstance()->cursor_x = current_x;
	FrameworkSingleton::getInstance()->cursor_y = current_y;

	glfwPollEvents();

	// Update the uniform buffer to allow for transforms to take place 
	vulkanManager.updateUniformBuffer(FrameworkSingleton::getInstance()->uniformBufferMemory);
	vulkanManager.updateUniformBuffer(FrameworkSingleton::getInstance()->rotatingUniformBufferMemory);
	// Draw the frame
	vulkanManager.drawFrame();
}