#include "../../include/physics_engine/physics_engine.h"

//Third party

#include <fmt/core.h>
#include <fmt/color.h>

PhysicsEngine::PhysicsEngine()
{
	fmt::print(fg(fmt::color::dark_salmon), "\n{}\n", "Physics Engine entry point.");
}

bool PhysicsEngine::init()
{
	// INIT ( to do )
	this->isInitialized = true;

	if(this->isInitialized)
		return true;
	
	return false;
}

void PhysicsEngine::run()
{
	if(this->init())
	{
		// Running Physics Engine
	}
}