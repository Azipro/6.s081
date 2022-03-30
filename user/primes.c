#include "kernel/types.h"
#include "user/user.h"

int isPrime(int);
int next_process(int);

int main(int argc, char* argv[]){
    int pipeline[2];
    pipe(pipeline);

    if(fork() == 0){
        close(pipeline[1]);
        next_process(pipeline[0]);
    }else{
        close(pipeline[0]);
        for(int i = 2 ; i <= 35; ++ i){
            write(pipeline[1], &i, sizeof(i));
        }
        close(pipeline[1]);
    }

    wait((int *) 0);
    exit(0);
}

int next_process(int get_father_input){
    int fd[2];
    pipe(fd);

    int base;
    if(read(get_father_input, &base, sizeof(base)) == 0){
        close(get_father_input);
        exit(0);
    }else{
        if(fork() == 0){
            close(fd[1]);
            next_process(fd[0]);
        }else{
            printf("prime %d\n", base);
            close(fd[0]);
            int i;
            while(read(get_father_input, &i, sizeof(i)) != 0){
                if(i % base) write(fd[1], &i, sizeof(i));
            }
            close(fd[1]);
        }
        wait((int *) 0);
    }
    exit(0);
}