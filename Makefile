SRC = $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp)
LIBS = -lfmt
main:
	g++ -std=c++23 -Wall $(SRC) -o collaboration $(LIBS)
	
clear:
	rm -rf collaboration
