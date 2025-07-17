#ifndef ANIMATION_ENGINE
#define ANIMATION_ENGINE

#include <iostream>

class AnimationEngine {

	bool isInitialized = false;
public:
	AnimationEngine();

	bool init();
	void run();

	int MainWindow();
};

#endif