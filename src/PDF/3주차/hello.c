#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    for (int i = 0; i < argc; i++) {
        printf("argc[%d]: %s\n", i, argv[i]);
    }

    return 0;
}