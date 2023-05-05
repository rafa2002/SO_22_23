#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
//#include <wait.h>
#include <limits.h>
#include <time.h>
#include "monitor.h"

int main(int argc, char ** argv){
    
    printf("O que o utilizador mandou executar - [");
    for(int a =0 ; a<argc;a++) printf(" %s",argv[a]);
    printf("]\n");
    
    char *arg = argv[3];
    int count = 0;
    char * token;
    char **comandos = malloc(sizeof(char*)*256);
    if(arg!=NULL)
    {   
        token = strtok(arg," ");
        while(token!=NULL){
            comandos[count] = malloc(sizeof(char *)*strlen(token)+1);
            comandos[count] = token;
            count++;
            //printf("O token é :%s\n",token);
            token = strtok(NULL," ");
        }
        comandos[count] = malloc(sizeof(NULL));
        comandos[count] = NULL;
        count++;
        
    }
    int canal = open("canal", O_WRONLY, 0666);
    char * message = malloc(sizeof(int) + sizeof(char)*300 + sizeof(time_t));
    int tamanho_str = 0, pid, mili_seconds_inicial = 0, mili_seconds_final = 0;
    
    if(argc>1 && strcmp(argv[1], "status")==0){
            if((pid=fork())==0){  
                pid = getpid();
            
                // Mensagem format: Running PID valor_pid valor_timestamp
                // Mensagem que o cliente escreve para o utilizador
                tamanho_str = sprintf(message,"STATUS %d\n",pid);
                write(1,message,tamanho_str);
                
                // Mensagem que o cliente escreve para o servidor
                write(canal,message,tamanho_str);
                int d = dup(canal);
                printf("valor do canal --- %d\n",d);
                printf("PID[%d] para executar: ",pid);
                
                char pid_strg[15];
                // ler a resposta
                sprintf(pid_strg,"%d",pid);
                
                int canal_status = open(pid_strg,O_RDONLY);
                char buf[2000];
                int bytes_lidos = 0;
                while(bytes_lidos = read(canal_status,&buf,2000*sizeof(char)))
                {
                    write(1,buf,sizeof(bytes_lidos));
                }
                close(canal);
                _exit(0);
            }else{
                wait(NULL);
                close(canal);
            }
        }
    else{
        if((pid=fork())==0){
            pid = getpid();
            //Este é o filho
            //Extrai o timestamp do inicio antes da execução
            struct timeval timestamp_inicial;
            gettimeofday(&timestamp_inicial, NULL);
            mili_seconds_inicial = timestamp_inicial.tv_usec/1000;
            
            // Mensagem format: Running PID valor_pid valor_timestamp
            // Mensagem que o cliente escreve para o utilizador
            /*char vazia[100];
            for(int tam = 1; tam<argc;tam++){
                strcat(vazia,argv[tam]);
            }*/
            tamanho_str = sprintf(message,"Running %d %d\n", pid, mili_seconds_inicial);
            strcat(message,"'");
            for(int tam = 1; tam<argc;tam++){
                strcat(message,argv[tam]);
            }
            strcat(message,"'\n");
            write(1,message,tamanho_str);
            // Mensagem que o cliente escreve para o servidor
            write(canal,message,tamanho_str);
            //free(vazia);
            int d = dup(canal);
            printf("valor do canal --- %d\n",d);
            printf("PID[%d] para executar: ",pid);
            execvp(comandos[0],comandos);
        }
        else{
            //Este é o pai
            wait(NULL);
            // Extrai o timestamp do fim após a execução
            struct timeval timestamp_final;
            gettimeofday(&timestamp_final, NULL);
            mili_seconds_final = timestamp_final.tv_usec/1000;

            // Mensagem que o cliente escreve para o servidor a avisar que concluiu
            tamanho_str = sprintf(message,"Running %s %d %d\n",argv, pid, mili_seconds_final);
            write(canal,message,tamanho_str);
            while(count>0){
                free(comandos[count]);
                count--;
            }
            free(comandos);
        }
    }
    free(message);
    // caso n haja mais pedidos fecha servidor
    return 0;
}
