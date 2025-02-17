#ifndef MAIN_ENTRY
#define MAIN_ENTRY

#include <iostream>
#include <cctype>
#include <map>


class MainEntry
{
private:
	bool isInitialized = false;
	char type;
	std::string title = "Collaboration v1.0\n\n";
	std::string questionEntry = "Which tool are you going to use? (select option [1-4] or q for quit): ";
	std::map<char, std::string> optionSet;

public:
	MainEntry();
	void showMessage(char type);
	bool init();
	void run();

	std::string separator(size_t width);
	std::string optionList();
};

#endif
