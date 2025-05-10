#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid == 0) {
        puts("Hi I am a child process");
        puts("End child process");
        return 1;
    } else {
        printf("Child Process ID: %d \n", pid);
        sleep(30);
        puts("End Parent Process");
    }

    return 0;
}