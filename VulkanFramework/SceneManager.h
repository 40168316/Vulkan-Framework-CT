#pragma once

#include "VulkanManager.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	VulkanManager vulkanManager;

	// Frames per second
	float FPS;
	// The time of a frame
	float frameTime;

	void input();
	void update();
};