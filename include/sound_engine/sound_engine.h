#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include <iostream>

class SoundEngine {
	bool isInitialized = false;

	bool sound_state = true;
public:
	SoundEngine();

	int MainWindow();

	bool init();
	void run();

	int sound();
};

#endif
