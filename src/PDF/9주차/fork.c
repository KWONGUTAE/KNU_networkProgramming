#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int gval = 10;
int main(int argc, char *argv[]) {
    pid_t pid;
    int lval = 20;

    lval += 5;

    pid = fork();
    if (pid == 0) {
        gval += 2, lval += 2;
        printf("Child Proc: [%d, %d], pid: %d\n", gval, lval, pid);
    } else {
        gval -= 2, lval -= 2;
        printf("Parent Proc: [%d, %d], pid: %d\n", gval, lval, pid);
    }

    return 0;
}