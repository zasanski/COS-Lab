#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid)
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        printf("Child: I am done. My pid: %d. My parent pid: %d.\n", getpid(), getppid());
        sleep(2);
        exit(EXIT_SUCCESS);
    default:
        printf("Parent: I am starting.\n");
        sleep(1);
        printf("Parent: I am done. My pid: %d. My parent pid: %d.\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }
}

/*
Scenario 1: Child finishes faster than parent:
- Child exits, becomes a zombie (due to SIGCHLD ignored).
- Parent exits.
- Init reaps the zombie.

Scenario 2: Parent finishes faster than child:
- Parent exits.
- Child becomes orphan, reparented to init.
- Child exits, init reaps it.
*/
