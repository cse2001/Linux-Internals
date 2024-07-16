#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

void sigchld_handler(int sig) {
    int status;
    pid_t soldier_pid = wait(&status);
    if (soldier_pid == -1) {
        perror("wait");
        exit(1);
    }
    printf("Soldier process %u terminated\n", soldier_pid);
}


int main() {

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }  
    printf("Control station process started with PID: %u\n", getpid());

    while (1) {
        sleep(1);
    }
    return 0;
}