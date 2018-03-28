#pragma once

#include "Camera.h"

namespace AllCamera
{
	class target_camera : public camera
	{
	public:
		//constructor
		target_camera() : camera() {}
		//copy constructor
		target_camera(const target_camera &other) = default;
		//assignment operator
		target_camera& operator=(const target_camera &rhs) = default;
		//destructor
		~target_camera() {}
		//Updator
		void update(float delta_time);
	};
}
