#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int status;
    pid_t pid = fork();
    int cid = 0;

    if (pid == 0) {
        sleep(15);
        return 24;
    } else {
        printf("Child pid = %d \n", pid);
        while ((cid = waitpid(-1, &status, WNOHANG)) == 0) 
        {
            sleep(3);
            puts("sleep 3sec.");
        }
        printf("exit while: cid= %d\n", cid);
        if (WIFEXITED(status)) {
            printf("Child sent %d \n", WEXITSTATUS(status));
        }
    }

    return 0;
}