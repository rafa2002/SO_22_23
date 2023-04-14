#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

int main(int argc, char ** argv){
    
    printf("O que o utilizador mandou executar - [%s]\n",argv);

    printf("argv[0] = %s,\n,argv[1] = %s\n",argv[0],argv[1]);

    char *arg = argv[2];
    int count = 0;
    char * token;
    if(arg!=NULL)
        {   
            count = 1;
            token = strtok(arg," ");
            while(token!=NULL){
                count++;
                printf("O token é :%s",token);
                token = strtok(NULL," ");
            }
        }
    for(int i = 1; i < count; i++){
        int pid;
        if((pid=fork())==0){
            //Este é o filho
            // armazenar_info_processo(int pid, char ** comando, struct tm *timestamp)
            char comandos[2] = {token[0],token[i]};
            time_t t = time(NULL);
            struct tm *timestamp = localtime(&t);
            armazenar_info_processo(pid,comandos,&timestamp);
            //Este printf é para imprimir no stdout o comando a executar informando assim o user
            printf("PID[%d] para executar %s %s\n",pid,token[0],token[i]);
            printf("O programa (token[0]) é: %s,\n e o argumento (token[%d]) é: %s",token[0],i,token[i]);
            execvp(token[0],token[i]);
        }
    }
    /*int password = open("/etc/passwd",O_RDONLY);
    if(password<0){
        perror("error on open passwd");
    }
    int saida = open("saida.txt",O_WRONLY | O_CREAT | O_TRUNC);
    if(saida<0){
        perror("error on open saida");
    }
    int err = open("error.txt",O_WRONLY | O_CREAT | O_TRUNC);
    if(err<0){    
        perror("error on open error");
    }

    int in = dup(0);
    int out = dup(1);
    int error = dup(2);
    
    dup2(password,0);
    dup2(saida,1);
    dup2(err,2);
    close(password);
    close(saida);
    close(err);

    int fd = fork();
    if(fd==0){
        int read_res;
        char buf[30];
        while((read_res = read(password,&buf,30))<=30){
        write(saida,&buf,read_res);
        }
    }
      
    

    dup2(out,1);
    write(1,"TERMINEI",8);
    */
    return 0;
}