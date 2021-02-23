#include "../include/start_point.hpp"

StartPoint::StartPoint()
{
	std::cout << this->question_what_tool;
	std::cin >> this->type;
	
	if(type >= 1 && type <= 5)
	{
		switch(type){
			case 1:
			{
				std::cout << "You selected " << type << " option." << std::endl;	
				std::cout << this->option_set[this->type - 1] << std::endl;
			}
			break;
			case 2:
			{
				std::cout << "You selected " << type << " option." << std::endl;	
				std::cout << this->option_set[this->type - 1] << std::endl;
			}
			break;
			case 3:
			{
				std::cout << "You selected " << type << " option." << std::endl;	
				std::cout << this->option_set[this->type - 1] << std::endl;
			}
			break;
			case 4:
			{
				std::cout << "You selected " << type << " option." << std::endl;	
				std::cout << this->option_set[this->type - 1] << std::endl;
			}
			break;
			case 5:
			{
				std::cout << "You selected " << type << " option." << std::endl;	
				std::cout << this->option_set[this->type - 1] << std::endl;
			}
		}
		
	}
	else
	{
		std::cout << "Out of option range." << std::endl;
	}	
}
