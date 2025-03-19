#ifndef PHYSICS_ENGINE
#define PHYSICS_ENGINE

#include <iostream>

class PhysicsEngine {

	bool isInitialized = false;
public:
	PhysicsEngine();

	bool init();
	void run();

	int MainWindow();
};

#endif
