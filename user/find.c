#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int find(char *,const char *);
void getFilename(char *, char *);

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(2, "Failed to find something, because argc=%d\n", argc);
    }

    char *path = argv[1];
    const char *dest = argv[2];

    if(find(path, dest) != 0){
        printf("Failed exec find.");
    }
    exit(0);
}

void getFilename(char *path, char *buf){
  char *p = path + strlen(path);
  while(p >= path && *p != '/') -- p;
  ++ p;
  memmove(buf, p, strlen(p));
}

int find(char *path, const char *s){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return -1;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    char* current_name = (char *)malloc(DIRSIZ + 1);
    getFilename(path, current_name);
    if(strcmp(current_name, s) == 0){
        printf("%s\n", path);
    }
    
    switch (st.type){
    case T_FILE:
        break;
    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
            printf("find: path too long\n");
            return -1;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0) continue;
            if(strcmp(de.name, ".\0") == 0 || strcmp(de.name, "..\0") == 0) continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            if(find(buf, s) != 0){
                return -1;
            }
        }
        break;
    }

    close(fd);
    return 0;
}