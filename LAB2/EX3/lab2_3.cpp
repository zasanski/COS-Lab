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

    for (int i = 0; i < 5; i++)
    {
        pid = fork();
        switch (pid)
        {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            printf("I am child. My pid: %d. My parent pid: %d.\n", getpid(), getppid());
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }
    printf("I am parent. My pid: %d. My parent pid: %d.\n", getpid(), getppid());
    exit(EXIT_SUCCESS);
}
