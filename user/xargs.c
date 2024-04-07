#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#define BUFSIZE 512
char buffer[BUFSIZE];

void clear_mem(char** args, int num) {
    for(int i = 0; i < num; ++i) {
        free(args[i]);
    }
}

int main(int argc, char* argv[]) {
    char* args[MAXARG];
    int nbytes;
    int buffer_index = 0;
    int arg_num = argc - 1;

    // check input arguments number    
    if(argc < 2) {
        fprintf(2, "Input arguments not enough: \
                    Usage: xargs <arguments> <command> <command-arguments>");
        exit(1);
    }

    // alloc memory for arguments
    for(int i = 0; i < MAXARG; ++i) {
        args[i] = malloc(BUFSIZE * sizeof(char));
        if(args[i] == 0) {
            fprintf(2, "Error occurs: memory not enough.");
            clear_mem(args, i);
            exit(1);
        }
    }

    // get <command arguments> for <command> after xargs
    for(int i = 1; i < argc; ++i) {
        strcpy(args[i-1], argv[i]);
    }

    while((nbytes = read(0, &buffer[buffer_index], 1)) > 0) {
        if(buffer[buffer_index] == '\n' || buffer_index == BUFSIZE - 1) {
            buffer[buffer_index] = 0;
            strcpy(args[arg_num++], buffer);
            int pid = fork();
            if(pid == 0) {
                args[arg_num] = 0;
                exec(argv[1], args);
                fprintf(2, "xargs: Unable to execute command : %s\n", argv[1]);
                clear_mem(args, MAXARG);
                exit(1);
            } else if (pid < 0) {
                fprintf(2, "xargs: Failed to fork.\n");
                clear_mem(args, MAXARG);
                exit(1);
            } else {
                wait(0);
                arg_num = argc - 1;
                buffer_index = 0;
            }
        } else {
            buffer_index++;
        }
    }

    clear_mem(args, MAXARG);
    exit(0);
}