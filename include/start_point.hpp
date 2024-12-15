#ifndef START_POINT_H_
#define START_POINT_H_

#include <iostream>
#include <cctype>
#include <map>

class StartPoint {
	char type;
	std::string title = "Collaboration v1.0\n\n";
	std::string optionList = "\nList of options\n"
							   "---------------------------------------------------------------------------\n"
							   "(1) Render Engine\t\t (2) Physics Engine\t\t (3) Collision Detection \n"
							   "(4) Sound Engine\t\t (5) Scripting\n"
							   "---------------------------------------------------------------------------\n\n";
	std::string questionEntry = "Which tool are you going to use? (select option [1-5] or q for quit): ";
	std::map<char, std::string> optionSet;
public:
	StartPoint();
	void showMessage(char type);
};

#endif
