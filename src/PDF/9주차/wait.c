#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int status;
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child process #1 is terminated(3) \n");
        return 3;
    } else {
        printf("Child #1 PID :%d \n", pid);
        pid = fork();

        if (pid == 0) {
            printf("Child process #2 is terminated(7)\n");
            exit(7);
        } else {
            printf("Child #2 PID: %d \n", pid);
            wait(&status);
            if (WIFEXITED(status)) {
                printf("Child #1 sent: %d \n", WEXITSTATUS(status));
            }

            wait(&status);
            if (WIFEXITED(status)) {
                printf("Child #2 sent: %d \n", WEXITSTATUS(status));
            }
            sleep(30);
        }
    }

    return 0;
}