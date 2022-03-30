#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int fw[2], sw[2];
    if(pipe(fw) < 0){
        fprintf(2, "Failed to create a pipe");
        exit(0);
    }
    if(pipe(sw) < 0){
        fprintf(2, "Failed to create a pipe");
        exit(0);
    }

    write(fw[1], "ping\n", 5);

    int pid = fork();
    if(pid == 0){
        char ping[512];
        write(sw[1], "pong\n", 5);
        read(fw[0], ping, 5);
        printf("%d: received %s", getpid(), ping);
    }else{
        wait((int*) 0);
        char pong[512];
        read(sw[0], pong, 5);
        printf("%d: received %s", getpid(), pong);
    }

    exit(0);
}