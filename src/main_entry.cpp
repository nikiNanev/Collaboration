#include "../include/main_entry.h"

#include "../include/render_engine/render_engine.h"
#include "../include/physics_engine/physics_engine.h"
#include "../include/sound_engine/sound_engine.h"
#include "../include/scripting/scripting.h"

// Third Party headers

// fmt
#include <fmt/core.h>
#include <fmt/color.h>

MainEntry::MainEntry()
{
	init();
}

bool MainEntry::init()
{
	this->optionSet['1'] = "Render Engine";
	this->optionSet['2'] = "Physics Engine";
	this->optionSet['3'] = "Sound Engine";
	this->optionSet['4'] = "Scripting";
	this->optionSet['q'] = "Quit";

	this->isInitialized = true; // INIT ( to do )

	if (this->isInitialized)
		return true;

	return false;
}

void MainEntry::run()
{
	if (this->init())
	{
		// Main Menu
		fmt::print(fg(fmt::color::bisque), "{0}\n{1}\n{2}",
				   this->title,
				   this->optionList,
				   this->questionEntry);

		std::cin >> this->type;

		if ((type >= '1' && type <= '4') || tolower(type) == 'q')
		{
			switch (type)
			{
			case '1':
			{
				// Render Engine
				this->showMessage(type);
				RenderEngine *pRender = new RenderEngine();
				pRender->run();
			}
			break;
			case '2':
			{
				// Physics Engine
				this->showMessage(type);
				PhysicsEngine *pPhysics = new PhysicsEngine();
				pPhysics->run();
			}
			break;
			case '3':
			{
				// Sound Engine
				this->showMessage(type);
				SoundEngine *pSounds = new SoundEngine();
				pSounds->run();
			}
			break;
			case '4':
			{
				// Scripting
				this->showMessage(type);
				Scripting *pScripting = new Scripting();
				pScripting->run();
			}
			break;
			case 'q':
			{
				// Quit
				this->showMessage(type);
			}
			}
		}
		else
		{
			fmt::print(fg(fmt::color::red), "Please, try another option.\n"); // Out of range option
		}
	}
}

void MainEntry::showMessage(char type)
{
	fmt::print(fg(fmt::color::blue_violet), "You select {0} option.\n", this->optionSet[type]);
}
