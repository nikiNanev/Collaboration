#include "include/main_entry.h"

int main(int argc, char **argv)
{
	MainEntry *pMainEntry = new MainEntry();
	pMainEntry->run();
	return 0;
}
