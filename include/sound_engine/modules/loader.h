#ifndef SOUND_ENGINE_LOADER_H
#define SOUND_ENGINE_LOADER_H

#pragma once

#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

class Loader
{

	private:

		bool sound_state = true;

	public:

		bool loadWAVFile(const std::string &filename, std::vector<char> &data, ALenum &format, ALsizei &freq);
		int sound(const std::string &filename);
		
		bool getState();
		void setState(bool state);

};
#endif
