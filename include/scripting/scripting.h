#ifndef SCRIPTING_MAIN
#define SCRIPTING_MAIN

#include <iostream>

class Scripting {
	bool isInitialized = false;
public:
	Scripting();

	bool init();
	void run();

	int MainWindow();
};

#endif
