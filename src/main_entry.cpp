#include "../include/main_entry.h"

#include "../include/render_engine/render_engine.h"
#include "../include/physics_engine/physics_engine.h"
#include "../include/sound_engine/sound_engine.h"
#include "../include/scripting/scripting.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <sstream>
#include <memory>

// Third Party headers

#include <fmt/core.h>
#include <fmt/color.h>

std::string MainEntry::Separator(size_t width)
{
	std::string separator = std::string(width, '-');
	return separator;
}

std::string MainEntry::getOptionsDescription()
{
	struct winsize w; // Structure for storing rows and cols of a terminal
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	std::string separator = this->Separator(w.ws_col - 2);

	std::string optionsText = "Render Engine { 1 | R }  Physics Engine { 2 | P }  Sound Engine { 3 | S }  Scripting Engine { 4 | C }  Quit { q | Q }";

	// Centering the options text ( terminalWidth - optionsTextLength ) / 2
	size_t gap = w.ws_col - optionsText.length();
	gap /= 2;
	std::string options = std::string(gap, ' ') + optionsText + std::string(gap, ' ');

	std::stringstream ssFormatter;
	ssFormatter << separator << std::endl;
	ssFormatter << options << std::endl;
	ssFormatter << separator << std::endl;

	std::string info = ssFormatter.str();

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
				   this->getOptionsDescription(),
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
			std::unique_ptr<RenderEngine> pRenderer = std::make_unique<RenderEngine>();
			pRenderer->run();
		}
		break;
		case '2':
		case 'P':
		case 'p':
		{
			// Physics Engine
			this->showMessage(type);
			std::unique_ptr<PhysicsEngine> pPhysics = std::make_unique<PhysicsEngine>();
			pPhysics->run();
		}
		break;
		case '3':
		case 'S':
		case 's':
		{
			// Sound Engine
			this->showMessage(type);

			std::unique_ptr<SoundEngine> pSounds = std::make_unique<SoundEngine>();
			pSounds->run();
		}
		break;
		case '4':
		case 'C':
		case 'c':
		{
			// Scripting
			this->showMessage(type);

			std::unique_ptr<Scripting> pScripting = std::make_unique<Scripting>();
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
