#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]) 
{
	if(argc < 2) {
		write(2, "Input parameter is missing, Usage: sleep <time in ms>\n", 55);
		exit(1);
	}
	int time = atoi(argv[1]);
	sleep(time);
	exit(0);
}
