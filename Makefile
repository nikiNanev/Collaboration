SRC = main.cpp src/render_engine/render_engine_main.cpp src/start_point.cpp \
	src/physics_engine/physics_engine_main.cpp src/collision_detection/collision_detection_main.cpp \
	src/sound_engine/sound_engine_main.cpp src/scripting/scripting_main.cpp

main:
	g++ -std=c++11 -o collaboration $(SRC)
	
clear:
	rm -rf collaboration
