#ifndef RENDER_ENGINE
#define RENDER_ENGINE

#include <iostream>
#include <iomanip>

class RenderEngine {
	bool isInitialized = false;
public:
	RenderEngine();
	
	bool init();
	void run();

	int MainWindow();
};

#endif
