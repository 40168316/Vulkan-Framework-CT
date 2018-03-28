// Include headers used for the application
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "FrameworkSingleton.h"

FrameworkSingleton* frameworkSingleton;

// Main method
int main() 
{
	frameworkSingleton = FrameworkSingleton::getInstance();

	frameworkSingleton->run();

	return EXIT_SUCCESS;
}