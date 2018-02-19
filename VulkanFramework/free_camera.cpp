#include "free_camera.h"

namespace AllCamera
{
	void free_camera::update(float delta_time)
	{
		// Calculate the forward direction - spherical coordinates to Cartesian
		glm::vec3 forward(cosf(pitch)* -sinf(yaw), sinf(pitch), -cosf(yaw) * cosf(pitch));
		//Normalize forward
		forward = glm::normalize(forward);

		//calculate standard right
		glm::vec3 right = glm::vec3(glm::eulerAngleY(yaw) * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		right = glm::normalize(right);

		up = glm::cross(right, forward);
		// Normalize up
		up = glm::normalize(up);

		glm::vec3 trans = translation.x * right;
		trans += translation.y * up;
		trans += translation.z * forward;
		posistion += trans;

		target = posistion + forward;

		translation = glm::vec3(.0f, .0f, .0f);

		view = glm::lookAt(posistion, target, up);

	}

	void free_camera::rotate(float delta_yaw, float delta_pitch)
	{
		// We  add rotation values and calculate new orientation
		pitch += delta_pitch;
		yaw -= delta_yaw;
	}

	void free_camera::move(const glm::vec3 &newtranslation)
	{
		//add translation vector to current translation
		translation += newtranslation;
	}
}
