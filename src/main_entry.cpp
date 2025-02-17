#include "../include/main_entry.h"

#include "../include/render_engine/render_engine.h"
#include "../include/physics_engine/physics_engine.h"
#include "../include/sound_engine/sound_engine.h"
#include "../include/scripting/scripting.h"

#include <sys/ioctl.h> // utilities for the terminal
#include <unistd.h>	   //STDOUT_FILENO

#include <sstream>

// Third Party headers

// fmt
#include <fmt/core.h>
#include <fmt/color.h>

struct winsize w; // terminal size ( width/height (rows/cols))

std::string MainEntry::separator(size_t width)
{
	std::string separator = std::string(width, '-');
	return separator;
}

std::string MainEntry::optionList()
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	std::stringstream ss;
	std::string separator = this->separator(w.ws_col - 2);

	std::string optionsText = "Render Engine { 1 | R }  Physics Engine { 2 | P }  Sound Engine { 3 | S }  Scripting Engine { 4 | C }  Quit { q | Q }";
	// Centering the text for options 
	size_t space = w.ws_col - optionsText.length();
	space /= 2;
	std::string options = std::string(space, ' ') + optionsText + std::string(space, ' ');

	ss << separator << std::endl;
	ss << options << std::endl;
	ss << separator << std::endl;

	std::string info = ss.str();

	return info;
}

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
				   this->optionList(),
				   this->questionEntry);

		std::cin >> this->type;

		switch (type)
		{
		case '1':
		case 'R':
		case 'r':
		{
			// Render Engine
			this->showMessage(type);
			RenderEngine *pRender = new RenderEngine();
			pRender->run();
		}
		break;
		case '2':
		case 'P':
		case 'p':
		{
			// Physics Engine
			this->showMessage(type);
			PhysicsEngine *pPhysics = new PhysicsEngine();
			pPhysics->run();
		}
		break;
		case '3':
		case 'S':
		case 's':
		{
			// Sound Engine
			this->showMessage(type);
			SoundEngine *pSounds = new SoundEngine();
			pSounds->run();
		}
		break;
		case '4':
		case 'C':
		case 'c':
		{
			// Scripting
			this->showMessage(type);
			Scripting *pScripting = new Scripting();
			pScripting->run();
		}
		break;
		case 'q':
		case 'Q':
		{
			// Quit
			this->showMessage(type);
		}
		break;
		default:
			fmt::print(fg(fmt::color::red), "Please, try another option.\n"); // Out of range option
		}
	}
}

void MainEntry::showMessage(char type)
{
	fmt::print(fg(fmt::color::blue_violet), "You select {0} option.\n", this->optionSet[type]);
}
