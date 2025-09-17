#ifndef SOUND_ENGINE
#define SOUND_ENGINE

class SoundEngine {

private:
	bool isInitialized = false;
public:
	SoundEngine();

	int MainWindow();

	bool init();
	void run();

};

#endif
