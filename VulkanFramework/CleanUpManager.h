#pragma once

class CleanUpManager
{
public:
	CleanUpManager();
	~CleanUpManager();

	void cleanup();
	void cleanupSwapChain();
};
