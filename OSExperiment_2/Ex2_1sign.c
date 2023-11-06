#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
int flag = 0;
void inter_handler() {
    flag = 0;
}
int main() {
    pid_t pid1=-1, pid2=-1;
    signal(3, inter_handler);
    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0) {
        while (pid2 == -1) pid2 = fork();
        if (pid2 > 0) {
            // TODO: 父进程
            flag = 1;
            sleep(5);
            kill(pid1, 16);
            kill(pid2, 17);
            wait(0);
            wait(0);
            printf("\nParent process is killed!!\n");
            exit(0);
        } 
        else {
            // TODO: 子进程 2
            flag = 1;
            signal(17, inter_handler);
            printf("\nChild process2 is killed by parent!!\n");
            return 0;
        }
    } 
    else {
        // TODO：子进程 1
        flag = 1;
        signal(16, inter_handler);
        printf("\nChild process1 is killed by parent!!\n");
        return 0;
    }
    return 0;
}