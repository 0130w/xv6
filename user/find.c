#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), 0, DIRSIZ-strlen(p));
  return buf;
}

void find(char* path, char* pattern) {
    int fd;
    struct stat st;
    char buf[512], *p;
    struct dirent de;
    if((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }

    if(fstat(fd, &st) < 0) {    
        // fstat retrieves information from inode that a fd refers to.
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    switch(st.type) {
        case T_DEVICE:
        case T_FILE:
            fprintf(2, "Path should be a path to a director.\n");
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long.\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                switch(st.type) {
                    case T_DEVICE:
                    case T_FILE:
                        if(!strcmp(fmtname(buf), pattern)) {
                            printf("%s\n", buf);
                        }
                        break;
                    case T_DIR:
                        if(!strcmp(fmtname(buf), ".") || !strcmp(fmtname(buf), ".."))
                            continue;
                        find(buf, pattern);
                        break;
                }
            }   
        break;
    }
    close(fd);
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("Input parameters not enough. Usage: find <path> <pattern>.\n");
        exit(1);
    }
    if(argc > 3) {
        printf("Input parameters too much. Usage: find <path> <pattern>.\n");
        exit(1);
    }
    char* pattern = 0;
    if(argc == 3) {
        pattern = argv[2];
    }
    find(argv[1], pattern);
    exit(0);
}