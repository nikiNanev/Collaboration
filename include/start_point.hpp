#ifndef START_POINT_H_
#define START_POINT_H_

#include <iostream>

class StartPoint {
	int type;
	std::string question_what_tool = "What tool are we going to create? (select option [1-5]): ";
	std::string option_set[5] = {
	"Render Engine",
	"Physics Engine",
	"Collision Detection",
	"Sound Engine",
	"Scripting"};
public:
	StartPoint();
	~StartPoint();
	void message(int type);
};

#endif
