#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <io.h>
// #include <unistd.h>
// #include <wait.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

int main(int argc, char ** argv){
    
    printf("O que o utilizador mandou executar - [%s]\n",argv);

    printf("argv[0] = %s,\n,argv[1] = %s\n",argv[0],argv[1]);
    
    char *arg = argv[2];
    int count = 0;
    char * token;
    char **comandos = malloc(sizeof(char*)*256); 
    if(arg!=NULL)
        {   
            token = strtok(arg," ");
            while(token!=NULL){
                comandos[count] = malloc(sizeof(char)*strlen(token)+1);
                comandos[count] = token;
                count++;
                printf("O token é :%s",token);
                token = strtok(NULL," ");

            }
        }
        int pid;
        if(strcmp(argv[0], "status")==0){
            pede_status();
        }
        else if((pid=fork())==0){
            //Este é o filho 
            // Extrai o timestamp do inicio antes da execução
            time_t t = time(NULL);
            struct tm *timestamp = localtime(&t);
            char * aux = "Running PID : ";
            char * pid_str = malloc(sizeof(int) + strlen(aux) + sizeof(time_t));
            //Mensagem que o cliente escreve para o cliente e para o servidor
            // Mensagem format: Running PID valor_pid valor_hora
            sprintf(pid_str,"Running PID %d %Y-%m-%d %H:%M:%S",pid,timestamp);
            int canal = mkfifo("canal.txt");
            // Escreve para o stdout a mensagem
            write(1,pid_str,strlen(pid_str));
            char copy = malloc(sizeof(char)*strlen(pid_str));
            strcpy(copy, pid_str);
            // Escreve para o servidor a mensagem
            write(canal,copy,strlen(pid_str));

            armazenar_info_processo(pid,comandos,timestamp);
            //Este printf é para imprimir no stdout o comando a executar informando assim o user
            printf("PID[%d] para executar: ",pid);
            for(int c = 1; c<size(comandos);c++){
                printf("%s ",comandos[c]);
                printf("\n");
            }
        execvp(comandos[0],comandos);
        // Extrai o timestamp do fim após a execução
        time_t tf = time(NULL);
        struct tm *timestampfinal = localtime(&tf);
        //double delta = (timestampfinal->tm_sec - timestamp->tm_sec) * 1000 + (timestampfinal->tm_msec - timestamp->tm_msec) / 1000;
        //int delta = difftime(timestamp,timestampfinal);
        
        char * aux = "Ended in ms";
        char * pid_str = malloc(sizeof(int) + strlen(aux));
        sprintf(pid_str,"Ended in %d ms",delta);
        write(1,pid_str,strlen(pid_str));

    }

            while(count>=0){
                free(comandos[count]);
                count--;
            }
            free(comandos);
            wait(NULL);
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