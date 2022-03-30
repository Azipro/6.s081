#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

char buf[1024];

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(2, "Failed to xargs something, because argc=%d\n", argc);
    }

    const int paramSize = argc - 1;
    char *command = argv[1];
    char *param[paramSize + 1];
    for(int i = 0 ; i < paramSize ; ++ i){
        int len = strlen(argv[i + 2]);
        param[i] = (char *)malloc(len);
        memmove(param[i], argv[i + 1], len);
    }
    param[paramSize] = (char *)malloc(1);

    int n, m;
    char *p, *q;
    m = 0;
    while((n = read(0, buf + m, sizeof(buf) - m - 1)) > 0){
        m += n;
        buf[m] = '\0';
        p = buf;
        while((q = strchr(p, '\n')) != 0){
            *q = 0;
            if(fork() == 0){
                free((void *)param[paramSize]);
                param[paramSize] = (char *)malloc(q + 1 - p);
                memmove(param[paramSize], p, q + 1 - p);
                // printf("command: %s paramSize: %d \n", command, paramSize);
                // printf("%s\n", param[paramSize]);
                exec(command, param);
                printf("Failed to exec %s\n", command);
            }
            wait((int *) 0);
            p = q + 1;
        }
        if(m > 0){ // 这一次从0中读取的不一定结尾有\n，还剩一些字符，放到下一个循环操作
            m -= p - buf;
            memmove(buf, p, m);
        }
    }

    for(int i = 0 ; i < paramSize + 1 ; ++ i) free(param[i]);

    exit(0);
}