#pragma once

#include "Camera.h"

namespace AllCamera
{
	class free_camera : public camera
	{
		private:
			//rotation on x-axis
			float pitch = 0.0f;
			// rotation on y-axis
			float yaw = 0.0f;
			// The translation of the camera since the last frame
			glm::vec3 translation;

		public:
			free_camera() : camera() {}
			~free_camera() {}
			// Updates the camera
			void update(float delta_time);


			float get_pitch() const { return pitch; }
			float get_yaw() const { return yaw; }

			void set_pitch(float value) { pitch = value; }
			void set_yaw(float value) { yaw = value; }

			// Rotates the camera by the given yaw (y-axis) and pitch (x-axis)
			void rotate(float delta_yaw, float delta_pitch);
			// Moves the camera
			void move(const glm::vec3 &translation);
	};
}
	