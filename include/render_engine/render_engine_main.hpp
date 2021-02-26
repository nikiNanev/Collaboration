#ifndef RENDER_ENGINE_MAIN_
#define RENDER_ENGINE_MAIN_

#include <iostream>
#include <iomanip>

#include "matrices/matrix_transformations.hpp"

class RenderEngineMain {
	size_t type_id;
public:
	RenderEngineMain(size_t type_id);
	~RenderEngineMain();
	
};

#endif
