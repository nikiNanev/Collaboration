#include "../include/start_point.hpp"

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
	std::cout << this->questionWhatTool;
	std::cin >> this->type;
	
	if(type >= '1' && type <= '5' || type == 'q')
	{
		switch(type){
			case '1':
			{
				//Render Engine
				this->showMessage(type);
			}
			break;
			case '2':
			{
				//Physics Engine
				this->showMessage(type);
			}
			break;
			case '3':
			{
				//Collision Detection
				this->showMessage(type);
			}
			break;
			case '4':
			{
				//Sound Engine
				this->showMessage(type);
			}
			break;
			case '5':
			{
				//Scripting
				this->showMessage(type);
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
