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

    printf("argv[0] = %s,\n,argv[1] = %s\nargv[2] = %s",argv[0],argv[1],argv[2]);
    
    char *arg = argv[3];
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
        int canal = open("canal", O_WRONLY, 0600);
        char * message = malloc(sizeof(int) + sizeof(char)*50 + sizeof(time_t));
        int tamanho_str = 0;

        int pid;
        if(strcmp(argv[1], "status")==0){
            pede_status();
        }
        else if((pid=fork())==0){
            //Este é o filho 
            // Extrai o timestamp do inicio antes da execução
            time_t t = time(NULL);
            struct tm *timestamp = localtime(&t);
            
            
            // Mensagem que o cliente escreve para o cliente e para o servidor
            // Mensagem format: Running PID valor_pid valor_hora
            tamanho_str = sprintf(message,"Running PID %d %Y-%m-%d %H:%M:%S",pid,timestamp);
            

            // Faço uma copia da string que quero escrever
            char copy = malloc(sizeof(char)*tamanho_str);
            strcpy(copy, tamanho_str);
            // Escreve para o servidor a mensagem
            write(canal,copy,tamanho_str);
            // Escreve para o stdout a mensagem
            write(1,message,tamanho_str);
            
            //Este printf é para imprimir no stdout o comando a executar informando assim o user
            printf("PID[%d] para executar: ",pid);
            for(int c = 1; c<size(comandos);c++){
                printf("%s ",comandos[c]);
                printf("\n");
            }

            execvp(comandos[0],comandos);
            
    }   
    else{
            //Este é o pai

            // Extrai o timestamp do fim após a execução
            time_t tf = time(NULL);
            struct tm *timestampfinal = localtime(&tf);

            tamanho_str = sprintf(message,"Ended PID %d %Y-%m-%d %H:%M:%S",pid,timestampfinal);
            write(1,message,tamanho_str);
            while(count>=0){
            free(comandos[count]);
            count--;
        }
        free(comandos);
        wait(NULL);
    }

        
    
    return 0;
}