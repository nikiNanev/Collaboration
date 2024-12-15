#include "../../include/render_engine/render_engine.h"


// Third party
#include <fmt/core.h>
#include <fmt/color.h>

RenderEngine::RenderEngine()
{
	fmt::print(fg(fmt::color::dark_red), "\n{}\n", "Render Engine Entry point.");
}

bool RenderEngine::init()
{
	// INIT ( to do )
	this->isInitialized = true;
	
	if(this->isInitialized)
		return true;

	return false;
}

void RenderEngine::run()
{
	// Everthing okey?
	if (this->init())
	{
		//Run this mother f***er ( lover )
	}
}