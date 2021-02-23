#include <stdio.h>
#include <stdlib.h>

//Set of Questions
const char *question_what_tool = "What tool are we going to create?";

int main(int argc, char **argv)
{
	int type;
	printf("%s: ", question_what_tool);
	scanf("%d", &type);
	
	printf("You selected %d type. \n", type);
	
	
	return 0;
}
