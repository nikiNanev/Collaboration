#include "../../include/scripting/scripting.h"

// Third party

#include <fmt/core.h>
#include <fmt/color.h>

Scripting::Scripting(){
	fmt::print(fg(fmt::color::dark_orange), "\n{}\n", "Scripting entry point.");
}

bool Scripting::init()
{
	//init ( to do )
	this->isInitialized = true;

	if(this->isInitialized)
		return true;
		
	return false;
}

void Scripting::run()
{
	if(this->init())
	{
		fmt::print(fg(fmt::color::dark_orange), "Run scripting, please!\n");
	}
}