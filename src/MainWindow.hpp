#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

class MainWindow
{

	public:
		MainWindow() : 
				windowWidth(800) ,
				windowHeight(600)
		{
			std::cout << "Main window!" << std::endl;

		}

		int run();

	private:
		size_t windowWidth, windowHeight;

};

#endif
