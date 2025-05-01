#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

int main()
{
#ifdef DEBUG
	printf("Compiled: %s at %s\n", __DATE__, __TIME__);
	printf("This is line %d of file %s\n", __LINE__, __FILE__);
#endif
	printf("Hello World\n");
	exit(0);
}

