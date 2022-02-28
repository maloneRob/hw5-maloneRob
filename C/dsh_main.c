#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>

#include"dsh.h"
int main(int argc, char** argv) {
    char *line=0;
    size_t size=0;

    dsh_init();
    printf("dsh> ");

    while(getline(&line,&size,stdin) > 0) {

        dsh_run(line);

        printf("dsh> ");
   }
}