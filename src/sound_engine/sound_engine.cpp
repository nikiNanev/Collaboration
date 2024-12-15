#include "../../include/sound_engine/sound_engine.h"

//Third party

#include <fmt/core.h>
#include <fmt/color.h>

SoundEngine::SoundEngine(){
	fmt::print(fg(fmt::color::sea_green), "\n{}\n", "Sound Engine entry point.");
}

bool SoundEngine::init()
{
	//Init ( to do )
	this->isInitialized = true;

	if(this->isInitialized)
		return true;
		
	return false;
}

void SoundEngine::run()
{
	if(this->init())
	{
		// Run Sounds, please
	}
}