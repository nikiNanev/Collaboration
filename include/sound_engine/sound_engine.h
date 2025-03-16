#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include <iostream>

class SoundEngine {
	bool isInitialized = false;
public:
	SoundEngine();

	int MainWindow();

	bool init();
	void run();
};

#endif
