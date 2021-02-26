#include "../../include/render_engine/render_engine_main.hpp"

RenderEngineMain::RenderEngineMain(size_t type_id){
	std::cout << "\nRender Engine Entry point.\n" << std::endl;
	std::cout << std::setfill('-') << std::setw(50) << "" << std::endl;
	std::cout << std::setfill(' ') << std::setw(30) << "ID: " << type_id << std::endl;
}
