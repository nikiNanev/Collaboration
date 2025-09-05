#include "main_entry.h"

#include "render_engine/render_engine.h"
#include "physics_engine/physics_engine.h"
#include "sound_engine/sound_engine.h"
#include "scripting/scripting.h"

#include <fmt/core.h>
#include <fmt/color.h>

int main(int argc, char *argv[])
{
	MainEntry *pMainEntry = new MainEntry();
	pMainEntry->run();
	return 0;
}
