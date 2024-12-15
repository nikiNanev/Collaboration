#include "../include/start_point.hpp"

#include "../include/render_engine/render_engine_main.hpp"
#include "../include/physics_engine/physics_engine_main.hpp"
#include "../include/collision_detection/collision_detection_main.hpp"
#include "../include/sound_engine/sound_engine_main.hpp"
#include "../include/scripting/scripting_main.hpp"

StartPoint::StartPoint()
{
	this->optionSet['1'] = "Render Engine";
	this->optionSet['2'] = "Physics Engine";
	this->optionSet['3'] = "Collision Detection";
	this->optionSet['4'] = "Sound Engine";
	this->optionSet['5'] = "Scripting";
	this->optionSet['q'] = "Quit";
	
	//Main Menu
	std::cout << this->title;
	std::cout << this->optionList;
	std::cout << this->questionEntry;
	std::cin >> this->type;
	
	
	
	if(type >= '1' && type <= '5' || tolower(type) == 'q')
	{
		switch(type){
			case '1':
			{
				//Render Engine
				this->showMessage(type);
				RenderEngineMain *rem = new RenderEngineMain(0);
			}
			break;
			case '2':
			{
				//Physics Engine
				this->showMessage(type);
				PhysicsEngineMain *pem = new PhysicsEngineMain();
			}
			break;
			case '3':
			{
				//Collision Detection
				this->showMessage(type);
				CollisionDetectionMain *cdm = new CollisionDetectionMain();
			}
			break;
			case '4':
			{
				//Sound Engine
				this->showMessage(type);
				SoundEngineMain *sem = new SoundEngineMain();
			}
			break;
			case '5':
			{
				//Scripting
				this->showMessage(type);
				ScriptingMain *sm = new ScriptingMain();
			}
			break;
			case 'q':
			{
				//Quit
				this->showMessage(type);
			}
		}
		
	}
	else
	{
		std::cout << "Out of option range." << std::endl;
	}	
}

void StartPoint::showMessage(char type)
{
	std::cout << "You selected " << type << " option." << std::endl;
	std::cout << this->optionSet[type] << std::endl;
}
