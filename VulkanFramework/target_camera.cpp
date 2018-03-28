#pragma once

#include "target_camera.h"

namespace AllCamera
{
	void target_camera::update(float delta_time)
	{
		//calculate front and side vector
		glm::vec3 forward = target - posistion;
		glm::vec3 side = glm::cross(up, forward);
		//use side and forward to calculate total
		glm::vec3 total = glm::cross(forward, side);
		total = glm::normalize(total);
		//set view
		view = glm::lookAt(posistion, target, up);
	}
}