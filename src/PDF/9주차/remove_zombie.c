#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int sig) {
    int status;
    pid_t id;
    /*
     pid_t id = waitpid(-1, &status, WNOHANG); 하면 자식 수거가 병합될 수도 있음.
     시그널 병합 때문에.
     근데 (== 0) 이면 자식이 종료되는게 없다는 말이니까, (>0) 이면 모든 자식을 순회하면서 수거할 수 있는 말.
     그래서 while문으로 사용한다.
    */
    while ((id = waitpid(-1, &status, WNOHANG)) > 0) { // 
        if(WIFEXITED(status)) {
            printf("Removed proc id: %d\n", id);
            printf("Child sent: %d \n", WEXITSTATUS(status));
        }
    }
}

int main(int argc, char *argv[]) {
    pid_t pid;
    struct sigaction act;

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    pid = fork();
    if (pid == 0) {
        puts("Hi! I'm child process #1");
        sleep(10);
        return 12;
    } else {
        printf("Child proc id: %d \n", pid);
        pid = fork();
        if (pid == 0) {
            puts("Hi! I'm child process #2");
            sleep(10);
            exit(24);
        } else {
            int i;
            printf("Child proc id: %d \n", pid);
            for (i = 0; i < 5; i++) {
                puts("Parent wait...");
                sleep(5);
            }
        }
    }

    return 0;
}