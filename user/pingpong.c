#include "kernel/types.h"
#include "user/user.h"

int main() {
    int p[2];
    pipe(p);
    int pid = fork();
    if(pid == 0) {
        read(p[0], 0, 1);
        printf("%d: received ping\n", getpid());
        write(p[1], "", 1);
    } else if (pid > 0) {
        write(p[1], "", 1);
        read(p[0], 0, 1);
        printf("%d: received pong\n", getpid());
    } else {
        fprintf(2, "fork failed\n");
        exit(1);
    }
    exit(0);
}