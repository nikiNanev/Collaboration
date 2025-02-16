// #include "include/main_entry.h"

#include "../include/main_entry.h"

#include "../include/render_engine/render_engine.h"
#include "../include/physics_engine/physics_engine.h"
#include "../include/sound_engine/sound_engine.h"
#include "../include/scripting/scripting.h"

// Third Party headers

// fmt
#include <fmt/core.h>
#include <fmt/color.h>

int main(int argc, char *argv[])
{
	MainEntry *pMainEntry = new MainEntry();
	pMainEntry->run();
	return 0;
}
