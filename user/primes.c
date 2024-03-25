#include "kernel/types.h"
#include "user/user.h"

void func(int* p) {
    close(p[1]);
    int n;
    if(read(p[0], &n, sizeof(int)) <= 0) {
        return;
    }
    printf("prime %d\n", n);
    int i;
    if(read(p[0], &i, sizeof(int)) <= 0) {
        return;
    }
    int p2[2];
    pipe(p2);
    int pid = fork();
    if(pid == 0){
        func(p2);
    } else if(pid < 0) {
        fprintf(2, "primes: Failed to fork.\n");
        exit(1);
    } else {
        close(p2[0]);
        do {
            if(i % n)
                write(p2[1], &i, sizeof(int));
        } while((read(p[0], &i, sizeof(int))) > 0);
        close(p[0]);
        close(p2[1]);
        wait((int *) 0);
    }
    return;
}

int main() {
    int p[2];
    pipe(p);
    int pid = fork();
    if(pid == 0) {
        func(p);
    } else if(pid < 0) {
        fprintf(2, "primes: Failed to fork.\n");
        exit(1);
    } else {
        close(p[0]);
        for(int i = 2; i <= 35; ++i) {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
        wait((int *) 0);
    }
    exit(0);
}